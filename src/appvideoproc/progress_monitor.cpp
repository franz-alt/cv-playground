#include "progress_monitor.hpp"

#include <iostream>

#include <boost/circular_buffer.hpp>

#include <boost/asynchronous/queue/lockfree_queue.hpp>
#include <boost/asynchronous/scheduler_shared_proxy.hpp>
#include <boost/asynchronous/scheduler/single_thread_scheduler.hpp>

struct progress_monitor::processing_context
{
    std::int64_t frames_total = 0;

    std::int64_t frames_load_done = 0;
    std::int64_t frames_load_failed = 0;

    std::int64_t frames_process_done = 0;
    std::int64_t frames_process_failed = 0;

    std::int64_t interframes_process_done = 0;
    std::int64_t interframes_process_failed = 0;

    std::int64_t frames_save_done = 0;
    std::int64_t frames_save_failed = 0;

    std::shared_ptr<boost::circular_buffer<cvpg::videoproc::update_indicator> > buffer;
};

progress_monitor::progress_monitor(boost::asynchronous::any_weak_scheduler<cvpg::imageproc::scripting::diagnostics::servant_job> scheduler,
                                   bool print_to_console,
                                   std::size_t buffer_per_context_entries)
    : boost::asynchronous::trackable_servant<cvpg::imageproc::scripting::diagnostics::servant_job, cvpg::imageproc::scripting::diagnostics::servant_job>(scheduler,
                                                                                                                                                         boost::asynchronous::create_shared_scheduler_proxy(
                                                                                                                                                             new boost::asynchronous::single_thread_scheduler<boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> >()
                                                                                                                                                         ))
    , m_print_to_console(print_to_console)
    , m_buffer_per_context_entries(buffer_per_context_entries)
    , m_contexts()
{
    if (m_print_to_console)
    {
        std::cout << "Progress:" << std::endl << "\n\n\n\n" << std::flush;
    }
}

void progress_monitor::init(std::size_t context_id, std::int64_t frames)
{
    // create new processing context
    auto context = std::make_shared<processing_context>();
    context->frames_total = frames;
    context->buffer = std::make_shared<boost::circular_buffer<cvpg::videoproc::update_indicator> >(m_buffer_per_context_entries);

    m_contexts.insert({ context_id, context });
}

void progress_monitor::finish(std::size_t context_id)
{
    m_contexts.erase(context_id);
}

void progress_monitor::update(std::size_t context_id, cvpg::videoproc::update_indicator update)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        if (update.context() == "load")
        {
            context->frames_load_done += update.processed();
            context->frames_load_failed += update.failed();
        }
        else if (update.context() == "frame")
        {
            context->frames_process_done += update.processed();
            context->frames_process_failed += update.failed();
        }
        else if (update.context() == "interframe")
        {
            context->interframes_process_done += update.processed();
            context->interframes_process_failed += update.failed();
        }
        else if (update.context() == "save")
        {
            context->frames_save_done += update.processed();
            context->frames_save_failed += update.failed();
        }

        context->buffer->push_back(std::move(update));

        if (m_print_to_console)
        {
            print_at_console(context_id);
        }
    }
}

void progress_monitor::print_at_console(std::size_t context_id)
{
    auto it = m_contexts.find(context_id);

    if (it != m_contexts.end())
    {
        auto & context = it->second;

        auto print_progress_bar =
            [](std::string title, double progress)
            {
                // TODO make the dynamic depending on current width of console
                const int bar_width = 70;

                std::cout << title << ": [";

                int pos = bar_width * progress;

                for (int i = 0; i < bar_width; ++i)
                {
                    if (i < pos)
                    {
                        std::cout << "=";
                    }
                    else if (i == pos)
                    {
                        std::cout << ">";
                    }
                    else
                    {
                        std::cout << " ";
                    }
                }

                std::cout << "] " << static_cast<int>(progress * 100.0) << " %\r" << std::flush << std::endl;
            };

        std::cout << "\e[A\e[A\e[A\e[A" << std::flush;
        
        print_progress_bar("- load  ", static_cast<double>(context->frames_load_done + context->frames_load_failed) / static_cast<double>(context->frames_total));
        print_progress_bar("- frames", static_cast<double>(context->frames_process_done + context->frames_process_failed) / static_cast<double>(context->frames_total - context->frames_load_failed));
        print_progress_bar("- inters", static_cast<double>(context->interframes_process_done + context->interframes_process_failed) / static_cast<double>(context->frames_total - context->frames_load_failed - 1));
        print_progress_bar("- save  ", static_cast<double>(context->frames_save_done + context->frames_save_failed) / static_cast<double>(context->frames_total - context->frames_load_failed - 1));
    }
}
