#include <libcvpg/videoproc/sinks/file.hpp>

#include <cstdint>

#include <boost/asynchronous/continuation_task.hpp>
#include <boost/asynchronous/queue/lockfree_queue.hpp>
#include <boost/asynchronous/scheduler_shared_proxy.hpp>
#include <boost/asynchronous/scheduler/single_thread_scheduler.hpp>

#include <boost/circular_buffer.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

#include <libcvpg/core/exception.hpp>

namespace cvpg::videoproc::sinks {

template<typename Image> struct file<Image>::processing_context
{
    bool prev_stage_finished = false;

    struct video_info
    {
        std::string uri;

        FILE * file = nullptr;

        AVFormatContext * format_context = nullptr;
        AVCodecContext * codec_context = nullptr;

        AVStream * stream = nullptr;

        std::int64_t stream_index = 0;

        std::int64_t frames_sent = 0;
        std::int64_t frames_received = 0;
    };

    video_info video;

    struct callback_info
    {
        std::function<void(std::map<std::string, std::any>)> params;
        std::function<void(std::size_t)> next;
        std::function<void(std::size_t)> finished;
    };

    callback_info callbacks;

    struct buffer_info
    {
        bool initialized = false;

        std::size_t next_frame = 0;

        struct entry
        {
            std::size_t id = 0;
            std::shared_ptr<std::uint8_t> frame;
            std::size_t size = 0;
        };

        std::shared_ptr<boost::circular_buffer<entry> > data;
    };

    buffer_info buffer;

    bool finished = false;
};

template<typename Image>
struct write_frames_task : public boost::asynchronous::continuation_task<std::vector<typename cvpg::videoproc::sinks::file<Image>::processing_context::buffer_info::entry> >
{
    write_frames_task(std::size_t context_id,
                      std::shared_ptr<typename cvpg::videoproc::sinks::file<Image>::processing_context> context,
                      videoproc::packet<videoproc::frame<Image> > packet)
        : boost::asynchronous::continuation_task<std::vector<typename cvpg::videoproc::sinks::file<Image>::processing_context::buffer_info::entry> >("sinks::file::write_frames")
        , m_context_id(context_id)
        , m_context(context)
        , m_packet(std::move(packet))
    {}

    void operator()()
    {
        AVFrame * frame = av_frame_alloc();

        if (!frame)
        {
            throw cvpg::exception("failed to allocate memory for frame");
        }

        frame->format = m_context->video.codec_context->pix_fmt;
        frame->width = m_context->video.codec_context->width;
        frame->height = m_context->video.codec_context->height;

        int ret = av_frame_get_buffer(frame, 0);

        if (ret < 0)
        {
            throw cvpg::exception("failed to allocate memory for video frame data");
        }

        AVPacket * packet = av_packet_alloc();

        if (!packet)
        {
            throw cvpg::exception("failed to allocate memory for packet");
        }

        std::vector<typename cvpg::videoproc::sinks::file<Image>::processing_context::buffer_info::entry> buffer;
        buffer.reserve(m_packet.frames().size());

        for (auto const & inter_frame : m_packet.frames())
        {
            // abort if frame is a flush frame
            if (inter_frame.flush())
            {
                continue;
            }

            ret = av_frame_make_writable(frame);

            if (ret < 0)
            {
                throw cvpg::exception("failed to allocate memory for video frame");
            }

            frame->pts = m_context->video.frames_sent++;

            auto image = inter_frame.image();

            std::int32_t ret = av_image_alloc(frame->data, frame->linesize, image.width(), image.height(), m_context->video.codec_context->pix_fmt, 32);

            if (ret < 0)
            {
                throw cvpg::exception("failed to allocate memory for raw picture buffer");
            }

            for (std::size_t y = 0; y < m_context->video.codec_context->height; y++)
            {
                for (std::size_t x = 0; x < m_context->video.codec_context->width; x++)
                {
                    std::uint8_t r = (&(*image.data(0)))[y * image.width() + x];
                    std::uint8_t g = (&(*image.data(1)))[y * image.width() + x];
                    std::uint8_t b = (&(*image.data(2)))[y * image.width() + x];

                    frame->data[0][y * frame->linesize[0] + x] = ((66 * r + 129 * g + 25 * b) >> 8) + 16;
                }
            }

            for (std::size_t y = 0; y < m_context->video.codec_context->height / 2; y++)
            {
                for (std::size_t x = 0; x < m_context->video.codec_context->width / 2; x++)
                {
                    std::uint8_t r = (&(*image.data(0)))[y * image.width() + x];
                    std::uint8_t g = (&(*image.data(1)))[y * image.width() + x];
                    std::uint8_t b = (&(*image.data(2)))[y * image.width() + x];

                    frame->data[1][y * frame->linesize[1] + x] = ((-38 * r + -74 * g + 112 * b) >> 8) + 128;
                    frame->data[2][y * frame->linesize[2] + x] = ((112 * r + -94 * g + -18 * b) >> 8) + 128;
                }
            }

            ret = avcodec_send_frame(m_context->video.codec_context, frame);

            if (ret < 0)
            {
                if (ret == AVERROR(EAGAIN))
                {
                    // TODO log error "AVERROR(EAGAIN)"
                }
                else if (ret == AVERROR(EINVAL))
                {
                    // TODO log error "AVERROR(EINVAL)"
                }
                else if (ret == AVERROR(ENOMEM))
                {
                    // TODO log error "AVERROR(ENOMEM)"
                }
                else if (ret == AVERROR_EOF)
                {
                    // TODO log error "AVERROR_EOF"
                }
                else
                {
                    // TODO log error "unknown error"
                }

                // TODO error handling
            }

            while (ret >= 0)
            {
                ret = avcodec_receive_packet(m_context->video.codec_context, packet);

                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                {
                    break;
                }
                else if (ret < 0)
                {
                    // TODO error handling

                    break;
                }

                typename cvpg::videoproc::sinks::file<Image>::processing_context::buffer_info::entry entry;
                entry.id = m_context->video.frames_received++;
                entry.size = static_cast<std::size_t>(packet->size * sizeof(std::uint8_t));

                // create memory from 'AVPacket' to separate memory
                entry.frame = std::shared_ptr<std::uint8_t>(static_cast<std::uint8_t *>(malloc(entry.size)), [](std::uint8_t * ptr){ free(ptr); });
                memcpy(entry.frame.get(), packet->data, entry.size);

                av_packet_unref(packet);

                buffer.push_back(std::move(entry));
            }

            av_freep(frame->data);

            // TODO indicate frame written
        }

        av_packet_free(&packet);
        av_frame_free(&frame);

        this->this_task_result().set_value(std::move(buffer));
    }

private:
    std::size_t m_context_id;

    std::shared_ptr<typename cvpg::videoproc::sinks::file<Image>::processing_context> m_context;

    videoproc::packet<videoproc::frame<Image> > m_packet;
};

template<typename Image>
boost::asynchronous::detail::callback_continuation<std::vector<typename cvpg::videoproc::sinks::file<Image>::processing_context::buffer_info::entry> >
write_frames(std::size_t context_id,
             std::shared_ptr<typename cvpg::videoproc::sinks::file<Image>::processing_context> context,
             videoproc::packet<videoproc::frame<Image> > packet)
{
    return boost::asynchronous::top_level_callback_continuation<std::vector<typename cvpg::videoproc::sinks::file<Image>::processing_context::buffer_info::entry> >(
               write_frames_task<Image>(context_id, context, std::move(packet))
           );
}

template<typename Image>
struct write_last_frames_task : public boost::asynchronous::continuation_task<std::vector<typename cvpg::videoproc::sinks::file<Image>::processing_context::buffer_info::entry> >
{
    write_last_frames_task(std::size_t context_id,
                           std::shared_ptr<typename cvpg::videoproc::sinks::file<Image>::processing_context> context)
        : boost::asynchronous::continuation_task<std::vector<typename cvpg::videoproc::sinks::file<Image>::processing_context::buffer_info::entry> >("sinks::file::write_lastframes")
        , m_context_id(context_id)
        , m_context(context)
    {}

    void operator()()
    {
        int ret = avcodec_send_frame(m_context->video.codec_context, nullptr);

        if (ret < 0)
        {
            throw cvpg::io_exception("error send frame to decoder");
        }

        AVPacket * packet = av_packet_alloc();

        if (!packet)
        {
            throw cvpg::exception("failed to allocate memmory for packet");
        }

        std::vector<typename cvpg::videoproc::sinks::file<Image>::processing_context::buffer_info::entry> buffer;
        buffer.reserve(100);

        while (ret >= 0)
        {
            ret = avcodec_receive_packet(m_context->video.codec_context, packet);

            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            {
                break;
            }
            else if (ret < 0)
            {
                // TODO error handling

                break;
            }

            typename cvpg::videoproc::sinks::file<Image>::processing_context::buffer_info::entry entry;
            entry.size = static_cast<std::size_t>(packet->size * sizeof(std::uint8_t));

            // create memory from 'AVPacket' to separate memory
            entry.frame = std::shared_ptr<std::uint8_t>(static_cast<std::uint8_t *>(malloc(entry.size)), [](std::uint8_t * ptr){ free(ptr); });
            memcpy(entry.frame.get(), packet->data, entry.size);

            av_packet_unref(packet);

            buffer.push_back(std::move(entry));
        }

        this->this_task_result().set_value(std::move(buffer));
    }

private:
    std::size_t m_context_id;

    std::shared_ptr<typename cvpg::videoproc::sinks::file<Image>::processing_context> m_context;
};

template<typename Image>
boost::asynchronous::detail::callback_continuation<std::vector<typename cvpg::videoproc::sinks::file<Image>::processing_context::buffer_info::entry>>
write_last_frames(std::size_t context_id,
                  std::shared_ptr<typename cvpg::videoproc::sinks::file<Image>::processing_context> context)
{
    return boost::asynchronous::top_level_callback_continuation<std::vector<typename cvpg::videoproc::sinks::file<Image>::processing_context::buffer_info::entry> >(
               write_last_frames_task<Image>(context_id, context)
           );
}

} // namespace cvpg::videoproc::sinks

namespace cvpg::videoproc::sinks {

template<typename Image> file<Image>::file(boost::asynchronous::any_weak_scheduler<imageproc::scripting::diagnostics::servant_job> scheduler, std::size_t buffered_frames)
    : boost::asynchronous::trackable_servant<imageproc::scripting::diagnostics::servant_job, imageproc::scripting::diagnostics::servant_job>(scheduler,
                                                                                                                                             boost::asynchronous::create_shared_scheduler_proxy(
                                                                                                                                                 new boost::asynchronous::single_thread_scheduler<boost::asynchronous::lockfree_queue<imageproc::scripting::diagnostics::servant_job> >()
                                                                                                                                             ))
    , m_buffered_frames(buffered_frames)
    , m_contexts()
{}

template<typename Image> void file<Image>::init(std::size_t context_id,
                                                std::string uri,
                                                std::function<void(std::size_t)> init_done_callback,
                                                std::function<void(std::size_t)> next_callback,
                                                std::function<void(std::size_t)> done_callback)
{
    // create new processing context
    auto context = std::make_shared<processing_context>();
    context->video.uri = std::move(uri);
    context->callbacks.next = std::move(next_callback);
    context->callbacks.finished = std::move(done_callback);
    context->buffer.data = std::make_shared<boost::circular_buffer<typename processing_context::buffer_info::entry> >(m_buffered_frames);

    m_contexts.insert({ context_id, context });

    context->video.file = fopen(context->video.uri.c_str(), "wb");

    if (!context->video.file)
    {
        throw cvpg::io_exception("failed to open output file");
    }

    context->video.format_context = avformat_alloc_context();

    if (context->video.format_context == nullptr)
    {
        throw cvpg::exception("failed to create output context");
    }

    // associate the output file (pointer) with the container format context
    AVIOContext * io_context = nullptr;
    context->video.format_context->pb = io_context;

    // guess the desired container format based on the file extension
    if (!(context->video.format_context->oformat = av_guess_format(nullptr, context->video.uri.c_str(), nullptr)))
    {
        avio_closep(&(context->video.format_context->pb));
        avformat_free_context(context->video.format_context);

        throw cvpg::exception("could not find output file format");
    }

    AVCodec * codec = avcodec_find_encoder(AVCodecID::AV_CODEC_ID_H264);

    // find the encoder to be used by its name
    if (codec == nullptr)
    {
        avio_closep(&(context->video.format_context->pb));
        avformat_free_context(context->video.format_context);

        throw cvpg::exception("could not find video decoder 'H264'");
    }

    // create a new audio stream in the output file container
    context->video.stream = avformat_new_stream(context->video.format_context, nullptr);

    if (context->video.stream == nullptr)
    {
        avio_closep(&(context->video.format_context->pb));
        avformat_free_context(context->video.format_context);

        throw cvpg::exception("could not create a new audio stream");
    }

    context->video.stream->first_dts = 0;
    context->video.stream->cur_dts = 0;

    context->video.stream_index = 0;

    context->video.codec_context = avcodec_alloc_context3(codec);

    if (context->video.codec_context == nullptr)
    {
        avio_closep(&(context->video.format_context->pb));
        avformat_free_context(context->video.format_context);

        throw cvpg::exception("could not allocate an encoding context");
    }

    init_done_callback(context_id);
}

template<typename Image> void file<Image>::params(std::size_t context_id, std::map<std::string, std::any> p)
{
    auto it = m_contexts.find(context_id);

    if (it == m_contexts.end())
    {
        throw cvpg::exception("no context with given ID found");
    }

    auto & context = it->second;

    // check frame width
    {
        auto it = p.find("frames.width");

        if (it != p.end())
        {
            context->video.codec_context->width = std::any_cast<std::size_t>(it->second);
        }
    }

    // check frame height
    {
        auto it = p.find("frames.height");

        if (it != p.end())
        {
            context->video.codec_context->height = std::any_cast<std::size_t>(it->second);
        }
    }

    if (context->video.codec_context->width == 0 || context->video.codec_context->height == 0)
    {
        throw cvpg::exception("invalid video dimensions");
    }

    context->video.codec_context->time_base = AVRational{1, 25}; // vinfo.codec_context->time_base;
    context->video.codec_context->pix_fmt = AVPixelFormat::AV_PIX_FMT_YUV420P;

    if (avcodec_open2(context->video.codec_context, context->video.codec_context->codec, nullptr) < 0)
    {
        avio_closep(&(context->video.format_context->pb));
        avformat_free_context(context->video.format_context);

        throw cvpg::io_exception("could not open video codec");
    }
}

template<typename Image> void file<Image>::start(std::size_t context_id)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        if (!(context->prev_stage_finished))
        {
            context->callbacks.next(context_id);
        }
    }
}

template<typename Image> void file<Image>::finish(std::size_t context_id)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        context->prev_stage_finished = true;
    }
}

template<typename Image> void file<Image>::process(std::size_t context_id, videoproc::packet<videoproc::frame<Image> > packet)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        bool is_flush_packet = packet.flush();

        post_callback(
            [context_id, context, packet = std::move(packet)]() mutable
            {
                return write_frames<Image>(context_id, context, std::move(packet));
            },
            [this, context_id, context, is_flush_packet](auto cont_res) mutable
            {
                try
                {
                    auto buffer = std::move(cont_res.get());

                    for (auto & entry : buffer)
                    {
                        context->buffer.data->push_back(std::move(entry));
                    }

                    this->try_flush_buffer(context_id);
                }
                catch (std::exception const & e)
                {
                    // TODO report error
                }
                catch (...)
                {
                    // TODO report error
                }
            },
            "sinks::file::process",
            1,
            1
        );

        if (is_flush_packet)
        {
            post_callback(
                [context_id, context]() mutable
                {
                    return write_last_frames<Image>(context_id, context);
                },
                [this, context_id, context](auto cont_res) mutable
                {
                    try
                    {
                        auto buffer = std::move(cont_res.get());

                        for (auto & entry : buffer)
                        {
                            context->buffer.data->push_back(std::move(entry));
                        }

                        this->try_flush_buffer(context_id);

                        // add sequence end code to have a real MPEG file
                        std::uint8_t endcode[] = { 0, 0, 1, 0xb7 };
                        fwrite(endcode, 1, sizeof(endcode), context->video.file);

                        // close file
                        fclose(context->video.file);

                        context->finished = true;
                        context->callbacks.finished(context_id);
                    }
                    catch (std::exception const & e)
                    {
                        // TODO report error
                    }
                    catch (...)
                    {
                        // TODO report error
                    }
                },
                "sinks::file::process::last_frame",
                1,
                1
            );
        }
    }
}

template<typename Image> void file<Image>::try_flush_buffer(std::size_t context_id)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        bool found = true;

        while (found)
        {
            found = false;

            if (!context->buffer.initialized && !context->buffer.data->empty())
            {
                context->buffer.initialized = true;
                context->buffer.next_frame = context->buffer.data->front().id;
            }

            for (auto it = context->buffer.data->begin(); it != context->buffer.data->end(); ++it)
            {
                auto & entry = *it;

                if (entry.id == context->buffer.next_frame)
                {
                    fwrite(entry.frame.get(), 1, entry.size, context->video.file);

                    context->buffer.data->erase(it);
                    context->buffer.next_frame++;

                    context->callbacks.next(context_id);

                    found = true;

                    break;
                }
            }
        }

        if (!found)
        {
            context->callbacks.next(context_id);
        }
    }
}

// manual instantation of file<> for some types
template class file<cvpg::image_gray_8bit>;
template class file<cvpg::image_rgb_8bit>;

} // namespace cvpg::videoproc::sinks
