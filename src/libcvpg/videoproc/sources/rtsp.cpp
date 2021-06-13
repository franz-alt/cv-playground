#include <libcvpg/videoproc/sources/rtsp.hpp>

#include <cstdint>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
}

#include <libcvpg/videoproc/stage_data_handler.hpp>
#include <libcvpg/videoproc/stage_fsm.hpp>

namespace {

#undef av_err2str
#define av_err2str(errnum) av_make_error_string((char*)__builtin_alloca(AV_ERROR_MAX_STRING_SIZE), AV_ERROR_MAX_STRING_SIZE, errnum)

template<typename Image>
int decode_packet(AVPacket * packet, AVCodecContext * codec_context, AVFrame * frame, std::vector<Image> & images)
{
    int res = avcodec_send_packet(codec_context, packet);

    if (res < 0)
    {
        return res;
    }

    while (res >= 0)
    {
        res = avcodec_receive_frame(codec_context, frame);

        if (res == AVERROR(EAGAIN) || res == AVERROR_EOF)
        {
            // TODO check if really not an error !?!?

            res = 0;

            av_frame_unref(frame);

            break;
        }
        else if (res < 0)
        {
            av_frame_unref(frame);

            break;
        }

        std::size_t channels = 0;
        AVPixelFormat pixel_format = AVPixelFormat::AV_PIX_FMT_NONE;

        if constexpr (std::tuple_size<typename Image::channel_array_type>::value == 1)
        {
            channels = 1;
            pixel_format = AVPixelFormat::AV_PIX_FMT_GRAY8;
        }
        else if constexpr (std::tuple_size<typename Image::channel_array_type>::value == 3)
        {
            channels = 3;
            pixel_format = AVPixelFormat::AV_PIX_FMT_RGB24;
        }
        else
        {
            // TODO handle error
        }

        Image image(frame->width, frame->height, 0);

        // create a SWC context to convert image from source to RGB pixel format
        auto sws_ctx = sws_getContext(codec_context->width,
                                      codec_context->height,
                                      codec_context->pix_fmt,
                                      image.width(),
                                      image.height(),
                                      pixel_format,
                                      0,
                                      0,
                                      0,
                                      0);

        AVFrame * dst = av_frame_alloc();
        dst->data[0] = new std::uint8_t[image.width() * image.height() * channels];
        dst->data[1] = nullptr;
        dst->linesize[0] = image.width() * channels;
        dst->linesize[1] = 0;

        // perform conversion to RGB raw data
        sws_scale(sws_ctx,
                  frame->data,
                  frame->linesize,
                  0,
                  image.height(),
                  dst->data,
                  dst->linesize);

        // convert into cvpg::image format
        for (std::size_t y = 0; y < image.height(); ++y)
        {
            std::size_t dst_pos = y * image.width();
            std::size_t src_pos = dst_pos * channels;

            for (std::size_t x = 0; x < image.width(); ++x, ++dst_pos, src_pos += channels)
            {
                if constexpr (std::tuple_size<typename Image::channel_array_type>::value == 1)
                {
                    image.data(0).get()[dst_pos] = dst->data[0][src_pos];
                }
                else if constexpr (std::tuple_size<typename Image::channel_array_type>::value == 3)
                {
                    image.data(0).get()[dst_pos] = dst->data[0][src_pos];
                    image.data(1).get()[dst_pos] = dst->data[0][src_pos + 1];
                    image.data(2).get()[dst_pos] = dst->data[0][src_pos + 2];
                }
            }
        }

        sws_freeContext(sws_ctx);

        av_frame_free(&dst);

        av_frame_unref(frame);

        images.push_back(std::move(image));
    }

    return res;
}

}

namespace cvpg::videoproc::sources {

template<typename Image> struct rtsp<Image>::processing_context
{
    struct frames_info
    {
        std::size_t width = 0;
        std::size_t height = 0;

        AVPixelFormat pixel_format = AVPixelFormat::AV_PIX_FMT_NONE;
    };

    frames_info frames;

    struct video_info
    {
        std::string uri;

        std::int64_t bit_rate = 0;
        std::int64_t duration = 0;
        std::int64_t frames = 0;

        AVCodecID codec_id = AVCodecID::AV_CODEC_ID_NONE;

        AVRational time_base = AVRational{1, 25}; // AVRational{0, 0};
        AVRational framerate = AVRational{25, 1}; // AVRational{0, 0};

        AVFormatContext * format_context = nullptr;
        AVCodecContext * codec_context = nullptr;

        std::size_t stream_index = 0;
    };

    video_info video;

    struct status_info
    {
        bool eof_reached = false;
        bool eof_flushed = false;

        std::size_t next_waiting = 0;

        std::size_t frames_loaded = 0;
        std::size_t frames_failed = 0;
        std::size_t frames_processed = 0;

        std::size_t packet_counter = 0;
    };

    status_info status;

    struct callback_info
    {
        std::function<void(std::size_t, std::map<std::string, std::any>)> params;
        std::function<void(std::size_t, videoproc::packet<videoproc::frame<Image> >)> deliver_packet;
        std::function<void(std::size_t)> finished;
        std::function<void(std::size_t, std::string)> failed;
        std::function<void(std::size_t, videoproc::update_indicator)> update_indicator;
    };

    callback_info callbacks;

    std::shared_ptr<stage_data_handler<videoproc::frame<Image> > > sdh;

    std::shared_ptr<videoproc::stage_fsm> fsm;
};

template<typename Image> rtsp<Image>::rtsp(boost::asynchronous::any_weak_scheduler<imageproc::scripting::diagnostics::servant_job> scheduler,
                                           std::size_t max_frames_read_buffer)
    : boost::asynchronous::trackable_servant<imageproc::scripting::diagnostics::servant_job, imageproc::scripting::diagnostics::servant_job>(scheduler)
    , m_max_frames_read_buffer(max_frames_read_buffer)
    , m_contexts()
{}

template<typename Image> void rtsp<Image>::init(std::size_t context_id, std::string uri, stage_callbacks<Image> callbacks)
{
    // create new processing context
    auto context = std::make_shared<processing_context>();
    context->video.uri = std::move(uri);
    context->callbacks.params = std::move(callbacks.parameters);
    context->callbacks.deliver_packet = std::move(callbacks.deliver);
    context->callbacks.finished = std::move(callbacks.finished);
    context->callbacks.failed = std::move(callbacks.failed);
    context->callbacks.update_indicator = std::move(callbacks.update);

    context->sdh = std::make_shared<stage_data_handler<videoproc::frame<Image> > >(
        "sources::rtsp",
        m_max_frames_read_buffer,
        [this, context_id, context]()
        {
            // start reading new data ...
            post_self(
                [this, context_id]()
                {
                    start(context_id);
                },
                "sources::rtsp::next",
                1
            );
        },
        [context]()
        {
            return context->status.next_waiting;
        },
        [this, context_id, context](std::vector<videoproc::frame<Image> > frames, std::function<void()> deliver_done_callback)
        {
            if (frames.empty())
            {
                deliver_done_callback();
                return;
            }

            context->status.next_waiting = 0;

            auto packet_number = context->status.packet_counter;

            videoproc::packet<videoproc::frame<Image> > packet(packet_number);

            bool is_last = false;

            for (auto & f : frames)
            {
                is_last |= f.flush();

                packet.add_frame(std::move(f));
            }

            context->callbacks.deliver_packet(context_id, std::move(packet));

            if (is_last)
            {
                context->status.eof_flushed = true;
                context->callbacks.finished(context_id);
            }
            else
            {
                deliver_done_callback();
            }
        },
        [context_id, context]()
        {
            context->callbacks.failed(context_id, "buffer full");
        }
    );

    context->fsm = std::make_shared<videoproc::stage_fsm>("sources::rtsp");

    m_contexts.insert({ context_id, context });

    avformat_network_init();

    context->video.format_context = avformat_alloc_context();
    avformat_open_input(&context->video.format_context, context->video.uri.c_str(), nullptr, nullptr);

    if (context->video.format_context == nullptr)
    {
        context->callbacks.failed(context_id, std::string("failed to open input '").append(context->video.uri).append("'"));

        return;
    }

    context->video.duration = context->video.format_context->duration;

    avformat_find_stream_info(context->video.format_context, nullptr);

    AVCodec * codec = nullptr;
    AVCodecParameters * codec_parameters = nullptr;

    for (std::size_t i = 0; i < context->video.format_context->nb_streams; i++)
    {
        AVCodecParameters * local_codec_parameters = context->video.format_context->streams[i]->codecpar;
        AVCodec * local_codec = avcodec_find_decoder(local_codec_parameters->codec_id);

        if (local_codec == nullptr)
        {
            context->callbacks.failed(context_id, "failed to open video codec");

            return;
        }

        // specific for video and audio
        if (local_codec_parameters->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            context->video.stream_index = i;
            codec = local_codec;
            codec_parameters = local_codec_parameters;

            context->frames.width = local_codec_parameters->width;
            context->frames.height = local_codec_parameters->height;
        }
        else if (local_codec_parameters->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            // TODO ignore this at moment
        }
        else
        {
            context->callbacks.failed(context_id, "unsupported codec type");

            return;
        }

        context->video.codec_id = local_codec->id;
        context->video.bit_rate = codec_parameters->bit_rate;
    }

    context->video.codec_context = avcodec_alloc_context3(codec);

    if (!context->video.codec_context)
    {
        avformat_close_input(&context->video.format_context);
        avformat_free_context(context->video.format_context);
        avcodec_free_context(&context->video.codec_context);

        context->callbacks.failed(context_id, "failed to allocate memory");

        return;
    }

    if (avcodec_parameters_to_context(context->video.codec_context, codec_parameters) < 0)
    {
        avformat_close_input(&context->video.format_context);
        avformat_free_context(context->video.format_context);
        avcodec_free_context(&context->video.codec_context);

        context->callbacks.failed(context_id, "failed to copy codec params to codec context");

        return;
    }

    context->frames.pixel_format = context->video.codec_context->pix_fmt;

    if (avcodec_open2(context->video.codec_context, codec, nullptr) < 0)
    {
        avformat_close_input(&context->video.format_context);
        avformat_free_context(context->video.format_context);
        avcodec_free_context(&context->video.codec_context);

        context->callbacks.failed(context_id, "failed to open codec");

        return;
    }

    // get amount of frames
    if (context->video.format_context->nb_streams > 0)
    {
        // TODO check if index 0 is the video stream or not
        const double fps = static_cast<double>(context->video.format_context->streams[context->video.stream_index]->avg_frame_rate.num) /
                           static_cast<double>(context->video.format_context->streams[context->video.stream_index]->avg_frame_rate.den);

        const std::int64_t frames = static_cast<std::int64_t>(static_cast<double>(context->video.format_context->duration) * fps / 1000000.0);

        context->video.frames = frames;
    }

    context->fsm->on_done(videoproc::stage_fsm::state_type::initializing,
                          [context_id, frames = context->video.frames, callback = std::move(callbacks.initialized)]()
                          {
                              callback(context_id, frames);
                          });

    // start reading packets from stream
    av_read_play(context->video.format_context);

    std::map<std::string, std::any> params =
    {
        { "frames.width", context->frames.width },
        { "frames.height", context->frames.height }
    };

    context->callbacks.params(context_id, std::move(params));

    context->fsm->process(videoproc::stage_fsm::event_type::initialize_done);
}

template<typename Image> void rtsp<Image>::params(std::size_t /*context_id*/, std::map<std::string, std::any> /*p*/)
{
    // not needed here!
}

template<typename Image> void rtsp<Image>::start(std::size_t context_id)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        // ignore start command if end of file is already reached
        if (context->status.eof_reached)
        {
            return;
        }

        // check if input buffer is full
        if (context->sdh->full())
        {
            // start reading new data ...
            // TODO perform post after a certain amount of time
            post_self(
                [this, context_id]()
                {
                    start(context_id);
                },
                "sources::rtsp::next",
                1
            );

            return;
        }

        std::vector<Image> images;
        images.reserve(m_max_frames_read_buffer);

        AVFrame * frame = av_frame_alloc();

        if (!frame)
        {
            context->callbacks.failed(context_id, "failed to allocate memory for frame");

            return;
        }

        AVPacket * packet = av_packet_alloc();

        if (!packet)
        {
            context->callbacks.failed(context_id, "failed to allocate memory for packet");

            return;
        }
        
        // try to read files to fill a quarter of the input buffer
        for (auto i = 0; i < context->sdh->free(); /* increment only when really encode */)
        {
            int res = av_read_frame(context->video.format_context, packet);

            if (res < 0)
            {
                if (res == AVERROR_EOF)
                {
                    context->status.eof_reached = true;
                }
                else
                {
                    context->callbacks.failed(context_id, "failed to read frame");

                    return;
                }

                break;
            }

            if (packet->stream_index == context->video.stream_index)
            {
                ++i;

                context->status.frames_loaded++;

                std::vector<Image> packet_images;

                if (decode_packet<Image>(packet, context->video.codec_context, frame, packet_images) < 0)
                {
                    context->status.frames_failed++;

                    context->callbacks.update_indicator(context_id, videoproc::update_indicator("load", 0, 1));
                }
                else
                {
                    images.insert(images.end(), packet_images.begin(), packet_images.end());

                    if (packet_images.empty())
                    {
                        // TODO indicate an empty packet in a separate way !?!?
                        context->callbacks.update_indicator(context_id, videoproc::update_indicator("load", 0, 1));
                    }
                    else
                    {
                        context->callbacks.update_indicator(context_id, videoproc::update_indicator("load", 1, 0));
                    }
                }
            }
        }

        av_frame_unref(frame);
        av_packet_unref(packet);

        av_frame_free(&frame);
        av_packet_free(&packet);

        if (!images.empty())
        {
            std::vector<videoproc::frame<Image> > frames;
            frames.reserve(images.size());

            for (auto & image : images)
            {
                frames.emplace_back(context->status.frames_processed++, std::move(image));
            }

            context->sdh->add(std::move(frames));

            if (context->status.eof_reached && !(context->status.eof_flushed))
            {
                // add flush frame when end-of-file reached
                context->sdh->add(videoproc::frame<Image>(context->status.frames_processed++));
            }
        }
    }
}

template<typename Image> void rtsp<Image>::finish(std::size_t /*context_id*/)
{
    // no finish needed here!
}

template<typename Image> void rtsp<Image>::process(std::size_t /*context_id*/, videoproc::packet<videoproc::frame<Image> > && /*packet*/)
{
    // no process needed here!
}

template<typename Image> void rtsp<Image>::next(std::size_t context_id, std::size_t max_new_data)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        context->status.next_waiting += max_new_data;

        context->sdh->try_flush();
    }
}

// manual instantiation of rtsp<> for some types
template class rtsp<cvpg::image_gray_8bit>;
template class rtsp<cvpg::image_rgb_8bit>;

} // namespace cvpg::videoproc::sources
