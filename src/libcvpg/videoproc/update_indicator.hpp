// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_VIDEOPROC_UPDATE_INDICATOR_HPP
#define LIBCVPG_VIDEOPROC_UPDATE_INDICATOR_HPP

#include <chrono>
#include <cstdint>
#include <string>

namespace cvpg::videoproc {

//
// Indicates an update in progress for a certain context. For each update a timestamp will be
// automatically generated when the object is created.
//
class update_indicator
{
public:
    update_indicator() = default;

    update_indicator(std::string context, std::int64_t processed, std::int64_t failed);

    std::chrono::time_point<std::chrono::high_resolution_clock> timestamp() const;

    std::string context() const;

    std::int64_t processed() const;
    std::int64_t failed() const;

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_timestamp;

    std::string m_context;

    std::int64_t m_processed = 0;
    std::int64_t m_failed = 0;
};

} // namespace cvpg::videoproc

#endif // LIBCVPG_VIDEOPROC_UPDATE_INDICATOR_HPP
