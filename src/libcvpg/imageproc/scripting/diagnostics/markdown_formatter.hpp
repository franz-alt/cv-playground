// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_DIAGNOSTICS_MARKDOWN_FORMATTER_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_DIAGNOSTICS_MARKDOWN_FORMATTER_HPP

#include <chrono>
#include <sstream>
#include <string>
#include <vector>

#include <boost/asynchronous/scheduler_diagnostics.hpp>
#include <boost/asynchronous/diagnostics/basic_formatter.hpp>
#include <boost/asynchronous/diagnostics/scheduler_interface.hpp>

namespace {

#define TICKS(block, key) item.durations.block.key.count()
#define DURATION(block, key) (item.count.block > 0 ? boost::asynchronous::formatting::format_duration(item.durations.block.key) : "-")

void format(std::stringstream & ss, std::size_t /* index */, std::string const & section, std::string const & name, boost::asynchronous::summary_diagnostics const & data)
{
    ss << "# " << name << std::endl;

    ss << "## Scheduling" << std::endl
       << "| Job | Total | Average | Max. | Min. | Count |" << std::endl
       << "| --- | ----: | ------: | ---: | ---: | ----: |" << std::endl;

    for (auto it = data.items.begin(); it != data.items.end(); ++it)
    {
        auto const & item = it->second;

        ss << "|" << item.job_name
           << "|" << DURATION(scheduling, total)
           << "|" << DURATION(scheduling, average)
           << "|" << DURATION(scheduling, max)
           << "|" << DURATION(scheduling, min)
           << "|" << item.count.scheduling
           << "|" << std::endl;
    }

    ss << std::endl;

    ss << "## Successful Execution" << std::endl
       << "| Job | Total | Average | Max. | Min. | Count |" << std::endl
       << "| --- | ----: | ------: | ---: | ---: | ----: |" << std::endl;

    for (auto it = data.items.begin(); it != data.items.end(); ++it)
    {
        auto const & item = it->second;

        ss << "|" << item.job_name
           << "|" << DURATION(success, total)
           << "|" << DURATION(success, average)
           << "|" << DURATION(success, max)
           << "|" << DURATION(success, min)
           << "|" << item.count.success
           << "|" << std::endl;
    }

    ss << std::endl;

    ss << "## Failure Execution" << std::endl
       << "| Job | Total | Average | Max. | Min. | Count |" << std::endl
       << "| --- | ----: | ------: | ---: | ---: | ----: |" << std::endl;

    for (auto it = data.items.begin(); it != data.items.end(); ++it)
    {
        auto const & item = it->second;

        ss << "|" << item.job_name
           << "|" << DURATION(failure, total)
           << "|" << DURATION(failure, average)
           << "|" << DURATION(failure, max)
           << "|" << DURATION(failure, min)
           << "|" << item.count.failure
           << "|" << std::endl;
    }

    ss << std::endl;
}

}

namespace cvpg { namespace imageproc { namespace scripting { namespace diagnostics {

struct parameters
{
    // TODO implement me
};

template<typename current = boost::asynchronous::scheduler_diagnostics, typename all = boost::asynchronous::summary_diagnostics>
class markdown_formatter : public virtual boost::asynchronous::basic_formatter<current, all>
{
public:
    typedef parameters parameter_type;

    // enable use of basic_formatter's diagnostics retrieval
    using boost::asynchronous::basic_formatter<current, all>::format;

    markdown_formatter(parameters params = parameters())
        : boost::asynchronous::basic_formatter<current, all>()
        , m_params(std::move(params))
    {}

    markdown_formatter(std::vector<boost::asynchronous::scheduler_interface> interfaces, parameters params = parameters())
        : boost::asynchronous::basic_formatter<current, all>(std::move(interfaces))
        , m_params(std::move(params))
    {}

    virtual std::string format(std::size_t count,
                               std::vector<std::string> && names,
                               std::vector<std::vector<std::size_t> > && /*queue_sizes*/,
                               std::vector<boost::asynchronous::scheduler_diagnostics::current_type> && /*running*/,
                               std::vector<current> && /*current*/,
                               std::vector<all> && all_) override
    {
        std::stringstream ss;

        for (std::size_t i = 0; i < count; ++i)
        {
            ::format(ss, i, "all", names[i], all_[i]);

            ss << std::endl;
        }

        return ss.str();
    }

    parameters & params()
    {
        return m_params;
    }

    parameters const & params() const
    {
        return m_params;
    }

protected:
    parameter_type m_params;
};

}}}} // namespace cvpg::imageproc::scripting::diagnostics

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_DIAGNOSTICS_MARKDOWN_FORMATTER_HPP
