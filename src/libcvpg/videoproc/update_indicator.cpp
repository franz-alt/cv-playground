// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/videoproc/update_indicator.hpp>

namespace cvpg::videoproc {

update_indicator::update_indicator(std::string context, std::int64_t processed, std::int64_t failed)
    : m_timestamp(std::chrono::high_resolution_clock::now())
    , m_context(std::move(context))
    , m_processed(processed)
    , m_failed(failed)
{}

std::string update_indicator::context() const
{
    return m_context;
}

std::int64_t update_indicator::processed() const
{
    return m_processed;
}

std::int64_t update_indicator::failed() const
{
    return m_failed;
}

} // namespace cvpg::videoproc
