#include <libcvpg/videoproc/processors/frame.hpp>

#include <exception>
#include <future>
#include <vector>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/exception.hpp>
#include <libcvpg/videoproc/stage_data_handler.hpp>

namespace cvpg::videoproc::processors {

template<typename Image> struct frame<Image>::processing_context
{
    std::size_t frames_id = 0;

    bool prev_stage_finished = false;

    struct status_info
    {
        std::size_t next_waiting = 0;

        std::size_t packet_counter = 0;
    };

    status_info status;

    struct callback_info
    {
        std::function<void(std::size_t, std::map<std::string, std::any>)> params;
        std::function<void(std::size_t, videoproc::packet<videoproc::frame<Image> >)> deliver_packet;
        std::function<void(std::size_t, std::size_t)> next;
        std::function<void(std::size_t)> finished;
        std::function<void(std::size_t, std::string)> failed;
        std::function<void(std::size_t, videoproc::update_indicator)> update_indicator;
    };

    callback_info callbacks;

    std::shared_ptr<stage_data_handler<videoproc::frame<Image> > > sdh_out;
};

template<typename Image> frame<Image>::frame(boost::asynchronous::any_weak_scheduler<imageproc::scripting::diagnostics::servant_job> scheduler,
                                             std::size_t max_frames_output_buffer,
                                             imageproc::scripting::image_processor_proxy image_processor)
    : boost::asynchronous::trackable_servant<imageproc::scripting::diagnostics::servant_job, imageproc::scripting::diagnostics::servant_job>(scheduler)
    , m_max_frames_output_buffer(max_frames_output_buffer)
    , m_image_processor(std::make_shared<imageproc::scripting::image_processor_proxy>(image_processor))
    , m_contexts()
{}

template<typename Image> void frame<Image>::init(std::size_t context_id, std::string script, stage_callbacks<Image> callbacks)
{
    // create new processing context
    auto context = std::make_shared<processing_context>();
    context->callbacks.params = std::move(callbacks.parameters);
    context->callbacks.deliver_packet = std::move(callbacks.deliver);
    context->callbacks.next = std::move(callbacks.next);
    context->callbacks.finished = std::move(callbacks.finished);
    context->callbacks.failed = std::move(callbacks.failed);
    context->callbacks.update_indicator = std::move(callbacks.update);

    context->sdh_out = std::make_shared<stage_data_handler<videoproc::frame<Image> > >(
        "frame",
        m_max_frames_output_buffer,
        [context_id, context]()
        {
            const auto free = context->sdh_out->free();

            if (free != 0)
            {
                // inform previous stage that this stage is ready to receive new data
                context->callbacks.next(context_id, free);
            }
        },
        [context]()
        {
            return context->status.next_waiting;
        },
        [this, context_id, context](std::vector<videoproc::frame<Image> > frames, std::function<void()> deliver_done_callback)
        {
            if (!frames.empty())
            {
                context->status.next_waiting = 0;

                auto packet_number = context->status.packet_counter;

                videoproc::packet<videoproc::frame<Image> > packet(packet_number);

                for (auto & f : frames)
                {
                    packet.add_frame(std::move(f));
                }

                context->callbacks.deliver_packet(context_id, std::move(packet));
            }

            deliver_done_callback();
        }
    );

    m_contexts.insert({ context_id, context });

    // compiling script
    struct compile_result
    {
        std::size_t id = 0;
        std::string error;
    };

    auto promise_compile = std::make_shared<std::promise<compile_result> >();

    std::vector<std::future<compile_result> > futures_compile;
    futures_compile.reserve(1);
    futures_compile.push_back(promise_compile->get_future());

    // compile frame script
    m_image_processor->compile(
        script,
        [promise_compile, context_id](std::size_t compile_id)
        {
            promise_compile->set_value({ compile_id });
        },
        [promise_compile, context_id](std::size_t compile_id, std::string error)
        {
            promise_compile->set_value({ compile_id, std::move(error) });
        }
    );

    boost::asynchronous::create_continuation_job<imageproc::scripting::diagnostics::diag_type>(
        [image_processor = m_image_processor, context, context_id, init_done_callback = std::move(callbacks.initialized)](auto cont_res) mutable
        {
            try
            {
                auto result = std::move(cont_res.at(0).get());

                context->frames_id = result.id;

                if (result.error.empty())
                {
                    init_done_callback(context_id, 0);
                }
                else
                {
                    throw cvpg::exception(result.error);
                }
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
        std::move(futures_compile)
    );
}

template<typename Image> void frame<Image>::params(std::size_t context_id, std::map<std::string, std::any> p)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        context->callbacks.params(context_id, std::move(p));
    }
}

template<typename Image> void frame<Image>::start(std::size_t context_id)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        if (!(context->prev_stage_finished))
        {
            context->callbacks.next(context_id, context->sdh_out->free());
        }
    }
}

template<typename Image> void frame<Image>::finish(std::size_t context_id)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        context->prev_stage_finished = true;
    }
}

template<typename Image> void frame<Image>::process(std::size_t context_id, videoproc::packet<videoproc::frame<Image> > && packet)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        process_next_frames(context_id, std::move(packet.move_frames()));

        context->sdh_out->try_flush();
    }
}

template<typename Image> void frame<Image>::next(std::size_t context_id, std::size_t max_new_data)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        context->status.next_waiting += max_new_data;

        context->sdh_out->try_flush();
    }
}

template<typename Image> void frame<Image>::process_next_frames(std::size_t context_id, std::vector<videoproc::frame<Image> > && frames)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        // update packet counter
        context->status.packet_counter++;

        // calling the image processor for each frame
        bool flush_frame = false;
        std::size_t flush_number = 0;

        for (auto const & frame : frames)
        {
            const auto frame_number = frame.number();

            if (frame.flush())
            {
                flush_frame = true;
                flush_number = frame_number;
                continue;
            }

            auto image = frame.image();

            m_image_processor->evaluate_convert_if(
                context->frames_id,
                std::move(image),
                make_safe_callback(
                    [context, context_id, frame_number](typename videoproc::frame<Image>::image_type image)
                    {
                        context->callbacks.update_indicator(context_id, videoproc::update_indicator("frame", 1, 0));

                        context->sdh_out->add(typename videoproc::frame<Image>(frame_number, std::move(image)));
                    },
                    "processors::frame::process_next_frames::callback::success",
                    1
                ),
                make_safe_callback(
                    [context](std::size_t context_id, std::string error)
                    {
                        context->callbacks.update_indicator(context_id, videoproc::update_indicator("frame", 0, 1));

                        context->callbacks.failed(context_id, std::move(error));
                    },
                    "processors::frame::process_next_frames::callback::failed",
                    1
                )
            );
        }

        if (flush_frame)
        {
            context->sdh_out->add(typename videoproc::frame<Image>(flush_number));
        }
    }
}

// manual instantiation of frame<> for some types
template class frame<cvpg::image_gray_8bit>;
template class frame<cvpg::image_rgb_8bit>;

} // namespace cvpg::videoproc::processors
