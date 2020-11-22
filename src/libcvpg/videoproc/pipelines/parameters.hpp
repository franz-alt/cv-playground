#ifndef LIBCVPG_VIDEOPROC_PIPELINES_PARAMETERS_HPP
#define LIBCVPG_VIDEOPROC_PIPELINES_PARAMETERS_HPP

#include <cstdint>
#include <functional>
#include <string>

#include <libcvpg/videoproc/update_indicator.hpp>

namespace cvpg::videoproc::pipelines::parameters {

struct uris
{
    std::string input;
    std::string output;
};

struct scripts
{
    std::string frame;
    std::string interframe;
};

struct callbacks
{
    std::function<void()> finished;
    std::function<void(std::size_t, std::int64_t)> init;
    std::function<void(std::size_t, std::string)> failed;
    std::function<void(std::size_t, cvpg::videoproc::update_indicator)> update;
};

} // cvpg::videoproc::pipelines::parameters

#endif // LIBCVPG_VIDEOPROC_PIPELINES_PARAMETERS_HPP
