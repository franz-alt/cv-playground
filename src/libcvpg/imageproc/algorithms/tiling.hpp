#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_HPP

#include <cstdint>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/image.hpp>

namespace cvpg { namespace imageproc { namespace algorithms {

enum class tiling_algorithms
{
    convert_to_gray,
    diff,
    mean,
    multiply_add
};

struct tiling_params
{
    tiling_algorithms algorithm;

    std::size_t cutoff_x = 0; // horizontal cutoff
    std::size_t cutoff_y = 0; // vertical cutoff

    double factor = 1.0;
    std::int32_t offset = 0;

    std::size_t image_width = 0;
    std::size_t image_height = 0;

    std::uint32_t filter_width = 0;
    std::uint32_t filter_height = 0;
};

boost::asynchronous::detail::callback_continuation<image_gray_8bit> tiling(image_gray_8bit image, tiling_params params);
boost::asynchronous::detail::callback_continuation<image_gray_8bit> tiling(image_gray_8bit image1, image_gray_8bit image2, tiling_params params);

boost::asynchronous::detail::callback_continuation<image_rgb_8bit> tiling(image_rgb_8bit image, tiling_params params);
boost::asynchronous::detail::callback_continuation<image_rgb_8bit> tiling(image_rgb_8bit image1, image_rgb_8bit image2, tiling_params params);

}}} // namespace cvpg::imageproc::algoritms

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_HPP
