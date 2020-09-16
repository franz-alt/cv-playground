#include <libcvpg/videoproc/processors/interframe.hpp>

#include <algorithm>
#include <deque>
#include <exception>
#include <future>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/exception.hpp>
#include <libcvpg/videoproc/stage_data_handler.hpp>

namespace {

template<typename Packet>
struct process_inter_frames_task : public boost::asynchronous::continuation_task<Packet>
{
    process_inter_frames_task(std::size_t context_id,
                              std::vector<typename Packet::frame_type> frames,
                              std::size_t packet_number,
                              std::size_t frames_offset,
                              std::shared_ptr<cvpg::imageproc::scripting::image_processor_proxy> image_processor,
                              std::size_t inter_frame_compile_id)
        : boost::asynchronous::continuation_task<Packet>("processors::interframe::process_inter_frames")
        , m_context_id(context_id)
        , m_frames(std::move(frames))
        , m_packet_number(packet_number)
        , m_frames_offset(frames_offset)
        , m_image_processor(image_processor)
        , m_inter_frame_compile_id(inter_frame_compile_id)
    {}

    void operator()()
    {
        try
        {
            auto images = std::make_shared<std::vector<typename Packet::frame_type::image_type> >();
            images->reserve(m_frames.size());

            std::vector<std::future<typename Packet::frame_type> > futures_evaluate;
            futures_evaluate.reserve(m_frames.size() - 1);

            bool flush_frame = false;
            std::size_t flush_number = 0;

            // extract/get raw images from items
            for (auto & frame : m_frames)
            {
                // abort if frame is a flush frame
                if (frame.flush())
                {
                    flush_frame = true;
                    flush_number = frame.number();
                    continue;
                }

                images->push_back(std::any_cast<typename Packet::frame_type::image_type>(frame.image()));
            }

            if (flush_frame)
            {
                // assume that we've 'lost' a single frame at interframe stage, so flush frame is one minus the last frame number
                // TODO find a more rubost solution here
                flush_number = m_frames.back().number() - 1;
            }

            if (images->empty())
            {
                // create a flush packet
                this->this_task_result().set_value(Packet(m_packet_number));

                return;
            }

            for (std::size_t i = 0; i < images->size() - 1; ++i)
            {
                auto promise_evaluate = std::make_shared<std::promise<typename Packet::frame_type> >();

                futures_evaluate.emplace_back(promise_evaluate->get_future());

                m_image_processor->evaluate_convert_if(
                    m_inter_frame_compile_id,
                    (*images)[i],
                    (*images)[i + 1],
                    [promise_evaluate, number = m_frames_offset + i, i, frames = m_frames.size() - 1, packet_number = m_packet_number](typename Packet::frame_type::image_type image)
                    {
                        promise_evaluate->set_value(typename Packet::frame_type(number, std::move(image)));
                    },
                    [promise_evaluate](std::size_t context_id, std::string error)
                    {
                        promise_evaluate->set_exception(std::make_exception_ptr(cvpg::exception(std::move(error))));
                    }
                );
            }

            boost::asynchronous::create_continuation_job<cvpg::imageproc::scripting::diagnostics::diag_type>(
                [task_res = this->this_task_result()
                ,image_processor = m_image_processor
                ,packet_number = m_packet_number
                ,images = std::move(images)
                ,flush_frame
                ,flush_number](auto cont_res)
                {
                    try
                    {
                        Packet packet(packet_number);

                        for (auto & res : cont_res)
                        {
                            packet.add_frame(std::move(res.get()));
                        }

                        if (flush_frame)
                        {
                            packet.add_frame(flush_number);
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
        catch (...)
        {
            this->this_task_result().set_exception(std::current_exception());
        }
    }

private:
    std::size_t m_context_id;

    std::vector<typename Packet::frame_type> m_frames;

    std::size_t m_packet_number;
    std::size_t m_frames_offset;

    std::shared_ptr<cvpg::imageproc::scripting::image_processor_proxy> m_image_processor;

    std::size_t m_inter_frame_compile_id;
};

template<typename Packet>
boost::asynchronous::detail::callback_continuation<Packet> process_inter_frames(std::size_t context_id,
                                                                                std::vector<typename Packet::frame_type> frames,
                                                                                std::size_t packet_number,
                                                                                std::size_t frames_offset,
                                                                                std::shared_ptr<cvpg::imageproc::scripting::image_processor_proxy> image_processor,
                                                                                std::size_t inter_frame_compile_id)
{
    return boost::asynchronous::top_level_callback_continuation<Packet>(
               process_inter_frames_task<Packet>(context_id, std::move(frames), packet_number, frames_offset, image_processor, inter_frame_compile_id)
           );
}

}

namespace cvpg::videoproc::processors {

template<typename Image> struct interframe<Image>::processing_context
{
    std::size_t frames_id = 0;

    bool prev_stage_finished = false;

    struct status_info
    {
        std::size_t next_waiting = 0;

        std::size_t packet_counter = 0;

        std::size_t packets_created = 0;
        std::size_t frames_created = 0;
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

    struct buffer_in_info
    {
        std::size_t next_frame = 0;
        std::shared_ptr<std::deque<videoproc::frame<Image> > > data;
    };

    buffer_in_info buffer_in;

    std::shared_ptr<stage_data_handler<videoproc::frame<Image> > > sdh_out;
};

template<typename Image> interframe<Image>::interframe(boost::asynchronous::any_weak_scheduler<imageproc::scripting::diagnostics::servant_job> scheduler,
                                                       boost::asynchronous::any_shared_scheduler_proxy<imageproc::scripting::diagnostics::servant_job> pool,
                                                       std::size_t max_frames_output_buffer,
                                                       imageproc::scripting::image_processor_proxy image_processor)
    : boost::asynchronous::trackable_servant<imageproc::scripting::diagnostics::servant_job, imageproc::scripting::diagnostics::servant_job>(scheduler, pool)
    , m_max_frames_output_buffer(max_frames_output_buffer)
    , m_image_processor(std::make_shared<imageproc::scripting::image_processor_proxy>(image_processor))
    , m_contexts()
{}

template<typename Image> void interframe<Image>::init(std::size_t context_id,
                                                      std::string script,
                                                      std::function<void(std::size_t)> init_done_callback,
                                                      std::function<void(std::size_t, std::map<std::string, std::any>)> params_callback,
                                                      std::function<void(std::size_t, videoproc::packet<videoproc::frame<Image> >)> packet_callback,
                                                      std::function<void(std::size_t, std::size_t)> next_callback,
                                                      std::function<void(std::size_t)> done_callback,
                                                      std::function<void(std::size_t, std::string)> failed_callback,
                                                      std::function<void(std::size_t, update_indicator)> update_indicator_callback)
{
    // create new processing context
    auto context = std::make_shared<processing_context>();
    context->callbacks.params = std::move(params_callback);
    context->callbacks.deliver_packet = std::move(packet_callback);
    context->callbacks.next = std::move(next_callback);
    context->callbacks.finished = std::move(done_callback);
    context->callbacks.failed = std::move(failed_callback);
    context->callbacks.update_indicator = std::move(update_indicator_callback);
    context->buffer_in.data = std::make_shared<std::deque<videoproc::frame<Image> > >();

    context->sdh_out = std::make_shared<stage_data_handler<videoproc::frame<Image> > >(
        "interframe",
        m_max_frames_output_buffer,
        [context_id, context]()
        {
            // inform previous stage that this stage is ready to receive new data
            context->callbacks.next(context_id, context->sdh_out->free());
        },
        [context]()
        {
            return context->status.next_waiting;
        },
        [this, context_id, context](std::vector<videoproc::frame<Image> > frames, std::function<void()> deliver_done_callback)
        {
            context->status.next_waiting = 0;

            auto packet_number = context->status.packet_counter;

            videoproc::packet<videoproc::frame<Image> > packet(packet_number);

            for (auto & f : frames)
            {
                packet.add_frame(std::move(f));
            }

            context->callbacks.deliver_packet(context_id, std::move(packet));

            deliver_done_callback();
        },
        [context_id, context]()
        {
            context->callbacks.failed(context_id, "buffer full");
        }
    );

    m_contexts.insert({ context_id, context });

    // compile interframe script
    struct compile_result
    {
        std::size_t id = 0;
        std::string error;
    };

    auto promise_compile = std::make_shared<std::promise<compile_result> >();

    std::vector<std::future<compile_result> > futures_compile;
    futures_compile.reserve(1);
    futures_compile.push_back(promise_compile->get_future());

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
                context->callbacks.failed(context_id, e.what());
            }
            catch (...)
            {
                context->callbacks.failed(context_id, "unknown error when processing interframes");
            }
        },
        std::move(futures_compile)
    );
}

template<typename Image> void interframe<Image>::params(std::size_t context_id, std::map<std::string, std::any> p)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        context->callbacks.params(context_id, std::move(p));
    }
}

template<typename Image> void interframe<Image>::start(std::size_t context_id)
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

template<typename Image> void interframe<Image>::finish(std::size_t context_id)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        context->prev_stage_finished = true;
    }
}

template<typename Image> void interframe<Image>::process(std::size_t context_id, videoproc::packet<videoproc::frame<Image> > packet)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        // add new frames at the end of the input buffer
        context->buffer_in.data->insert(context->buffer_in.data->end(), packet.frames().begin(), packet.frames().end());

        try_process_input(context_id);
    }
}

template<typename Image> void interframe<Image>::next(std::size_t context_id, std::size_t max_new_data)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        context->status.next_waiting += max_new_data;

        context->sdh_out->try_flush();
    }
}

template<typename Image> void interframe<Image>::try_process_input(std::size_t context_id)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        if (context->buffer_in.data->empty())
        {
            return;
        }

        std::vector<std::size_t> numbers;
        numbers.reserve(context->buffer_in.data->size());

        for (auto & frame : *context->buffer_in.data)
        {
            numbers.push_back(frame.number());
        }

        std::sort(numbers.begin(), numbers.end());

        std::size_t frame_number = context->buffer_in.next_frame;

        bool found = true;

        // TODO speed up this code!
        while (found)
        {
            found = false;

            auto it = std::find_if(numbers.begin(),
                                   numbers.end(),
                                   [frame_number](std::size_t number)
                                   {
                                       return number == frame_number;
                                   });

            if (it != numbers.end())
            {
                found = true;
                ++frame_number;
            }
            else
            {
                --frame_number;
            }
        }

        std::vector<videoproc::frame<Image> > frames;
        frames.reserve(context->buffer_in.data->size());

        // TODO use partition instead of copy_if here?!?!
        std::copy_if(context->buffer_in.data->begin(),
                     context->buffer_in.data->end(),
                     std::back_inserter(frames),
                     [frame_number](auto const & frame)
                     {
                         return frame.number() <= frame_number;
                     });

        std:sort(frames.begin(),
                 frames.end(),
                 [](auto const & a, auto const & b)
                 {
                     return a.number() < b.number();
                 });

        // except the frame, delete all older ones
        for (auto it = context->buffer_in.data->begin(); it != context->buffer_in.data->end(); )
        {
            if (it->number() <= (frame_number - 1))
            {
                it = context->buffer_in.data->erase(it);
            }
            else
            {
                ++it;
            }
        }

        context->buffer_in.next_frame = frame_number;

        auto frames_amount = frames.size();

        auto packets_created = context->status.packets_created;
        auto frames_created = context->status.frames_created;

        post_callback(
            [context_id
            ,image_processor = m_image_processor
            ,frames = std::move(frames)
            ,packets_created
            ,frames_created
            ,frames_id = context->frames_id]()
            {
                return process_inter_frames<videoproc::packet<videoproc::frame<Image> > >(
                           context_id,
                           std::move(frames),
                           packets_created,
                           frames_created,
                           image_processor,
                           frames_id
                       );
            },
            [this, context_id, packet_number = packets_created, context](auto cont_res)
            {
                try
                {
                    auto packet = std::move(cont_res.get());

                    context->callbacks.update_indicator(context_id, videoproc::update_indicator("interframe", packet.frames().size(), 0));

                    context->sdh_out->add(std::move(packet.move_frames()));
                }
                catch (std::exception const & e)
                {
                    context->callbacks.failed(context_id, e.what());
                }
                catch (...)
                {
                    context->callbacks.failed(context_id, "unknown error when processing interframes");
                }
            },
            "processors::interframe::try_process_input",
            1,
            1
        );

        context->status.packets_created++;
        context->status.frames_created += frames_amount - 1;
    }
}

// manual instantiation of interframe<> for some types
template class interframe<cvpg::image_gray_8bit>;
template class interframe<cvpg::image_rgb_8bit>;

} // namespace cvpg::videoproc::processors
