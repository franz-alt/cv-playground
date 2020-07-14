#include <libcvpg/videoproc/processors/frame.hpp>

#include <exception>
#include <future>
#include <vector>

#include <boost/circular_buffer.hpp>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/exception.hpp>

namespace {

template<typename Packet>
struct process_frames_task : public boost::asynchronous::continuation_task<Packet>
{
    process_frames_task(std::size_t context_id,
                        std::size_t packet_number,
                        std::vector<typename Packet::frame_type> frames,
                        std::shared_ptr<cvpg::imageproc::scripting::image_processor_proxy> image_processor,
                        std::size_t frame_compile_id)
        : boost::asynchronous::continuation_task<Packet>("processors::frame::process_frames")
        , m_context_id(context_id)
        , m_packet_number(packet_number)
        , m_frames(std::move(frames))
        , m_image_processor(image_processor)
        , m_frame_compile_id(frame_compile_id)
    {}

    void operator()()
    {
        std::vector<std::future<typename Packet::frame_type> > futures_evaluate;
        futures_evaluate.reserve(m_frames.size());

        for (auto & frame : m_frames)
        {
            auto image = frame.image();

            auto promise_evaluate = std::make_shared<std::promise<typename Packet::frame_type> >();

            futures_evaluate.emplace_back(promise_evaluate->get_future());

            m_image_processor->evaluate_convert_if(
                m_frame_compile_id,
                std::move(image),
                [promise_evaluate = std::move(promise_evaluate), number = frame.number(), packet_number = m_packet_number](typename Packet::frame_type::image_type image) mutable
                {
                    promise_evaluate->set_value(typename Packet::frame_type(number, std::move(image)));
                }
            );
        }

        boost::asynchronous::create_continuation_job<cvpg::imageproc::scripting::diagnostics::diag_type>(
            [task_res = this->this_task_result()
            ,context_id = m_context_id
            ,packet_number = m_packet_number
            ,image_processor = m_image_processor](auto cont_res) mutable
            {
                try
                {
                    Packet packet(packet_number);

                    for (auto & res : cont_res)
                    {
                        packet.add_frame(std::move(res.get()));
                    }

                    task_res.set_value(std::move(packet));
                }
                catch (...)
                {
                    task_res.set_exception(std::current_exception());
                }
            },
            std::move(futures_evaluate)
        );
    }

private:
    std::size_t m_context_id;

    std::size_t m_packet_number;

    std::vector<typename Packet::frame_type> m_frames;

    std::shared_ptr<cvpg::imageproc::scripting::image_processor_proxy> m_image_processor;

    std::size_t m_frame_compile_id;
};

template<typename Packet>
boost::asynchronous::detail::callback_continuation<Packet> process_frames(std::size_t context_id,
                                                                          std::size_t packet_number,
                                                                          std::vector<typename Packet::frame_type> frames,
                                                                          std::shared_ptr<cvpg::imageproc::scripting::image_processor_proxy> image_processor,
                                                                          std::size_t frame_compile_id)
{
    return boost::asynchronous::top_level_callback_continuation<Packet>(
               process_frames_task<Packet>(context_id, packet_number, std::move(frames), image_processor, frame_compile_id)
           );
}

}

namespace cvpg::videoproc::processors {

template<typename Image> struct frame<Image>::processing_context
{
    std::size_t frames_id = 0;

    bool prev_stage_finished = false;

    struct callback_info
    {
        std::function<void(std::size_t, std::map<std::string, std::any>)> params;
        std::function<void(std::size_t, videoproc::packet<videoproc::frame<Image> >)> deliver_packet;
        std::function<void(std::size_t)> next;
        std::function<void(std::size_t)> finished;
        std::function<void(std::size_t, videoproc::update_indicator)> update_indicator;
    };

    callback_info callbacks;

    std::shared_ptr<boost::circular_buffer<videoproc::packet<videoproc::frame<Image> > > > buffer;
};

template<typename Image> frame<Image>::frame(boost::asynchronous::any_weak_scheduler<imageproc::scripting::diagnostics::servant_job> scheduler,
                                             boost::asynchronous::any_shared_scheduler_proxy<imageproc::scripting::diagnostics::servant_job> pool,
                                             std::size_t buffered_packets,
                                             imageproc::scripting::image_processor_proxy image_processor)
    : boost::asynchronous::trackable_servant<imageproc::scripting::diagnostics::servant_job, imageproc::scripting::diagnostics::servant_job>(scheduler, pool)
    , m_buffered_packets(buffered_packets)
    , m_image_processor(std::make_shared<imageproc::scripting::image_processor_proxy>(image_processor))
    , m_contexts()
{}

template<typename Image> void frame<Image>::init(std::size_t context_id,
                                                 std::string script,
                                                 std::function<void(std::size_t)> init_done_callback,
                                                 std::function<void(std::size_t, std::map<std::string, std::any>)> params_callback,
                                                 std::function<void(std::size_t, videoproc::packet<videoproc::frame<Image> >)> packet_callback,
                                                 std::function<void(std::size_t)> next_callback,
                                                 std::function<void(std::size_t)> done_callback,
                                                 std::function<void(std::size_t, update_indicator)> update_indicator_callback)
{
    // create new processing context
    auto context = std::make_shared<processing_context>();
    context->callbacks.params = std::move(params_callback);
    context->callbacks.deliver_packet = std::move(packet_callback);
    context->callbacks.next = std::move(next_callback);
    context->callbacks.finished = std::move(done_callback);
    context->callbacks.update_indicator = std::move(update_indicator_callback);
    context->buffer = std::make_shared<boost::circular_buffer<videoproc::packet<videoproc::frame<Image> > > >(m_buffered_packets);

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
        [image_processor = m_image_processor, context, context_id, init_done_callback](auto cont_res) mutable
        {
            try
            {
                auto result = std::move(cont_res.at(0).get());

                context->frames_id = result.id;

                if (result.error.empty())
                {
                    init_done_callback(context_id);
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
            context->callbacks.next(context_id);
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

template<typename Image> void frame<Image>::process(std::size_t context_id, videoproc::packet<videoproc::frame<Image> > packet)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        if (packet.flush())
        {
            // add 'flush packet' at end of buffer
            context->buffer->push_back(std::move(packet));

            try_flush_buffer(context_id);
        }
        else
        {
            std::vector<videoproc::frame<Image> > frames(packet.move_frames());

            post_callback(
                [context_id
                ,packet_number = packet.number()
                ,frames = std::move(frames)
                ,image_processor = m_image_processor
                ,frame_compile_id = context->frames_id]()
                {
                    return process_frames<videoproc::packet<videoproc::frame<Image> > >(
                               context_id,
                               packet_number,
                               std::move(frames),
                               image_processor,
                               frame_compile_id
                           );
                },
                [this, context_id, context](auto cont_res) mutable
                {
                    try
                    {
                        auto packet = std::move(cont_res.get());

                        context->callbacks.update_indicator(context_id, videoproc::update_indicator("frame", packet.frames().size(), 0));

                        context->buffer->push_back(std::move(packet));

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
                "processors::frame::process",
                1,
                1
            );
        }
    }
}

template<typename Image> void frame<Image>::next(std::size_t context_id)
{
    try_flush_buffer(context_id);
}

template<typename Image> void frame<Image>::try_flush_buffer(std::size_t context_id)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        if (context->buffer->empty())
        {
            // inform previous stage that this stage is ready to receive new data
            context->callbacks.next(context_id);
        }
        else
        {
            auto & packet = context->buffer->front();

            bool is_last = packet.flush();

            // deliver packet to next stage
            context->callbacks.deliver_packet(context_id, std::move(packet));

            context->buffer->pop_front();

            // if 'flush packet' is at buffer, inform next stage of finishing for this context
            if (is_last)
            {
                context->callbacks.finished(context_id);
            }
            else
            {
                // inform previous stage that this stage is ready to receive new data
                context->callbacks.next(context_id);
            }
        }
    }
}

// manual instantation of frame<> for some types
template class frame<cvpg::image_gray_8bit>;
template class frame<cvpg::image_rgb_8bit>;

} // namespace cvpg::videoproc::processors
