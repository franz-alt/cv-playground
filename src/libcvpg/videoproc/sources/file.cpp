#include <libcvpg/videoproc/sources/file.hpp>

#include <cstdint>

#include <boost/circular_buffer.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

#include <libcvpg/core/exception.hpp>
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

        Image image(frame->linesize[0], frame->height, 0);

        // TODO use correct RGB frames here !!!
        memcpy(image.data(0).get(), frame->data[0], frame->linesize[0] * frame->height);
        memcpy(image.data(1).get(), frame->data[0], frame->linesize[0] * frame->height);
        memcpy(image.data(2).get(), frame->data[0], frame->linesize[0] * frame->height);

        av_frame_unref(frame);

        images.push_back(std::move(image));
    }

    return res;
}

}

namespace cvpg::videoproc::sources {

template<typename Image> struct file<Image>::processing_context
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
        std::function<void(std::size_t, videoproc::update_indicator)> update_indicator;
    };

    callback_info callbacks;

    std::size_t buffered_frames = 0;
    std::size_t buffered_packets = 0;

    std::shared_ptr<boost::circular_buffer<videoproc::packet<videoproc::frame<Image> > > > buffer;

    std::shared_ptr<videoproc::stage_fsm> fsm;
};

template<typename Image> file<Image>::file(boost::asynchronous::any_weak_scheduler<imageproc::scripting::diagnostics::servant_job> scheduler,
                                           std::size_t buffered_frames,
                                           std::size_t buffered_packets)
    : boost::asynchronous::trackable_servant<imageproc::scripting::diagnostics::servant_job, imageproc::scripting::diagnostics::servant_job>(scheduler)
    , m_buffered_frames(buffered_frames)
    , m_buffered_packets(buffered_packets)
    , m_contexts()
{}

template<typename Image> void file<Image>::init(std::size_t context_id,
                                                std::string uri,
                                                std::function<void(std::size_t, std::int64_t)> init_done_callback,
                                                std::function<void(std::size_t, std::map<std::string, std::any>)> params_callback,
                                                std::function<void(std::size_t, videoproc::packet<videoproc::frame<Image> >)> packet_callback,
                                                std::function<void(std::size_t)> done_callback,
                                                std::function<void(std::size_t, update_indicator)> update_indicator_callback)
{
    // create new processing context
    auto context = std::make_shared<processing_context>();
    context->video.uri = std::move(uri);
    context->callbacks.params = std::move(params_callback);
    context->callbacks.deliver_packet = std::move(packet_callback);
    context->callbacks.finished = std::move(done_callback);
    context->callbacks.update_indicator = std::move(update_indicator_callback);
    context->buffered_frames = m_buffered_frames;
    context->buffered_packets = m_buffered_packets;
    context->buffer = std::make_shared<boost::circular_buffer<videoproc::packet<videoproc::frame<Image> > > >(m_buffered_packets);
    context->fsm = std::make_shared<videoproc::stage_fsm>("sources::file");

    m_contexts.insert({ context_id, context });

    // open video
    context->video.format_context = avformat_alloc_context();
    avformat_open_input(&context->video.format_context, context->video.uri.c_str(), nullptr, nullptr);

    if (context->video.format_context == nullptr)
    {
        throw cvpg::io_exception(std::string("failed to open input '").append(context->video.uri).append("'"));
    }

    context->video.duration = context->video.format_context->duration;

    avformat_find_stream_info(context->video.format_context, nullptr);

    AVCodec * codec = nullptr;
    AVCodecParameters * codec_parameters = nullptr;

    for (std::size_t i = 0; i < context->video.format_context->nb_streams; ++i)
    {
        AVCodecParameters * local_codec_parameters = context->video.format_context->streams[i]->codecpar;
        AVCodec * local_codec = avcodec_find_decoder(local_codec_parameters->codec_id);

        if (local_codec == nullptr)
        {
            throw cvpg::exception("failed to open video codec");
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
        else // if (local_codec_parameters->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            throw cvpg::exception("unsupported codec type");
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

        throw cvpg::exception("failed to allocate memory");
    }

    if (avcodec_parameters_to_context(context->video.codec_context, codec_parameters) < 0)
    {
        avformat_close_input(&context->video.format_context);
        avformat_free_context(context->video.format_context);
        avcodec_free_context(&context->video.codec_context);

        throw cvpg::exception("failed to copy codec params to codec context");
    }

    context->frames.pixel_format = context->video.codec_context->pix_fmt;

    // get amount of frames
    if (context->video.format_context->nb_streams > 0)
    {
        // TODO check if index 0 is the video stream or not

        context->video.frames = context->video.format_context->streams[0]->nb_frames;
    }

    context->fsm->on_done(videoproc::stage_fsm::state_type::initializing,
                          [context_id, frames = context->video.frames, callback = std::move(init_done_callback)]()
                          {
                              callback(context_id, frames);
                          });

    if (avcodec_open2(context->video.codec_context, codec, nullptr) < 0)
    {
        avformat_close_input(&context->video.format_context);
        avformat_free_context(context->video.format_context);
        avcodec_free_context(&context->video.codec_context);

        throw cvpg::io_exception("failed to open codec");
    }

    std::map<std::string, std::any> params =
    {
        { "frames.width", context->frames.width },
        { "frames.height", context->frames.height }
    };

    context->callbacks.params(context_id, std::move(params));

    context->fsm->process(videoproc::stage_fsm::event_type::initialize_done);
}

template<typename Image> void file<Image>::start(std::size_t context_id)
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

        // check if packet buffer is full
        if (context->buffer->size() == context->buffered_packets)
        {
            // don't read new data until packet buffer is freed
            // TODO do we have to throw away old data in case of streaming mode !?!?!
            return;
        }

        std::vector<Image> images;
        images.reserve(context->buffered_frames);

        AVFrame * frame = av_frame_alloc();

        if (!frame)
        {
            throw cvpg::exception("failed to allocate memory for frame");
        }

        AVPacket * packet = av_packet_alloc();

        if (!packet)
        {
            throw cvpg::exception("failed to allocate memory for packet");
        }

        for (auto i = 0; i < context->buffered_frames; ++i)
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
                    throw cvpg::io_exception("failed to read frame");
                }

                break;
            }

            if (packet->stream_index == context->video.stream_index)
            {
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
            else
            {
                context->status.frames_failed++;

                context->callbacks.update_indicator(context_id, videoproc::update_indicator("load", 0, 1));
            }
        }

        av_frame_unref(frame);
        av_packet_unref(packet);

        av_frame_free(&frame);
        av_packet_free(&packet);

        videoproc::packet<videoproc::frame<Image> > packet_(context->status.packet_counter++);

        for (auto & image : images)
        {
            packet_.add_frame(context->status.frames_processed++, std::move(image));
        }

        context->buffer->push_back(std::move(packet_));

        if (context->status.eof_reached)
        {
            // add 'flush packet' at end-of-file
            videoproc::packet<videoproc::frame<Image> > packet_(context->status.packet_counter++);
            packet_.add_frame(context->status.frames_processed++);

            context->buffer->push_back(std::move(packet_));
        }

        try_flush_buffer(context_id);
    }
}

template<typename Image> void file<Image>::next(std::size_t context_id)
{
    try_flush_buffer(context_id);
}

template<typename Image> void file<Image>::try_flush_buffer(std::size_t context_id)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        // ignore flush when end-of-file already flushed
        if (context->status.eof_flushed)
        {
            return;
        }

        if (!context->buffer->empty())
        {
            auto & packet = context->buffer->front();

            const bool is_last = packet.flush();

            // deliver packet to next stage
            context->callbacks.deliver_packet(context_id, std::move(packet));

            context->buffer->pop_front();

            // if 'flush packet' is at buffer, inform next stage of finishing for this context
            if (is_last)
            {
                context->status.eof_flushed = true;
                context->callbacks.finished(context_id);
            }
        }

        // start reading new data ...
        post_self(
            [this, context_id]()
            {
                start(context_id);
            },
            "sources::file::next",
            1
        );
    }
}

// manual instantation of file<> for some types
template class file<cvpg::image_gray_8bit>;
template class file<cvpg::image_rgb_8bit>;

} // namespace cvpg::videoproc::sources
