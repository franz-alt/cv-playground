#ifndef LIBCVPG_VIDEOPROC_STAGE_PARAMETERS_HPP
#define LIBCVPG_VIDEOPROC_STAGE_PARAMETERS_HPP

#include <any>
#include <cstdint>
#include <functional>
#include <map>
#include <string>

#include <libcvpg/videoproc/frame.hpp>
#include <libcvpg/videoproc/packet.hpp>
#include <libcvpg/videoproc/update_indicator.hpp>

namespace cvpg::videoproc {

template<class Image>
struct stage_callbacks
{
    std::function<void(std::size_t, std::int64_t)> initialized;
    std::function<void(std::size_t, std::map<std::string, std::any>)> parameters;
    std::function<void(std::size_t, videoproc::packet<videoproc::frame<Image> >)> deliver;
    std::function<void(std::size_t, std::size_t)> next;
    std::function<void(std::size_t)> finished;
    std::function<void(std::size_t, std::string)> failed;
    std::function<void(std::size_t, update_indicator)> update;
};

} // namespace cvpg::videoproc

#endif // LIBCVPG_VIDEOPROC_STAGE_PARAMETERS_HPP
