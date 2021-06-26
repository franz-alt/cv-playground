// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef PROGRESS_MONITOR_HPP
#define PROGRESS_MONITOR_HPP

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include <boost/asynchronous/servant_proxy.hpp>
#include <boost/asynchronous/trackable_servant.hpp>

#include <libcvpg/imageproc/scripting/diagnostics/typedefs.hpp>
#include <libcvpg/videoproc/update_indicator.hpp>

class progress_monitor : public boost::asynchronous::trackable_servant<cvpg::imageproc::scripting::diagnostics::servant_job, cvpg::imageproc::scripting::diagnostics::servant_job>
{
public:
    progress_monitor(boost::asynchronous::any_weak_scheduler<cvpg::imageproc::scripting::diagnostics::servant_job> scheduler,
                     bool print_to_console = true,
                     std::size_t buffer_per_context_entries = 1000000);

    progress_monitor(progress_monitor const &) = delete;
    progress_monitor(progress_monitor &&) = delete;

    progress_monitor & operator=(progress_monitor const &) = delete;
    progress_monitor & operator=(progress_monitor &&) = delete;

    virtual ~progress_monitor() = default;

    void init(std::size_t context_id, std::int64_t frames);
    void update(std::size_t context_id, cvpg::videoproc::update_indicator update);
    void finish(std::size_t context_id);

private:
    void print_at_console(std::size_t context_id);

    bool m_print_to_console;

    std::size_t m_buffer_per_context_entries;

    struct processing_context;
    std::map<std::size_t, std::shared_ptr<processing_context> > m_contexts;
};

struct progress_monitor_proxy : public boost::asynchronous::servant_proxy<progress_monitor_proxy, progress_monitor, cvpg::imageproc::scripting::diagnostics::servant_job>
{
   template<typename... Args>
   progress_monitor_proxy(Args... args)
       : boost::asynchronous::servant_proxy<progress_monitor_proxy, progress_monitor, cvpg::imageproc::scripting::diagnostics::servant_job>(std::forward<Args>(args)...)
   {}

   BOOST_ASYNC_POST_MEMBER_LOG(init, "init", 1)
   BOOST_ASYNC_POST_MEMBER_LOG(update, "update", 1)
   BOOST_ASYNC_POST_MEMBER_LOG(finish, "finish", 1)
};

#endif // PROGRESS_MONITOR_HPP
