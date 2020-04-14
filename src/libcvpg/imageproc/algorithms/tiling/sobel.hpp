#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_SOBEL_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_SOBEL_HPP

#include <cstdint>

#include <libcvpg/imageproc/algorithms/tiling/parameters.hpp>

namespace cvpg { namespace imageproc { namespace algorithms {

enum class sobel_operation_mode
{
    horizontal,
    vertical
};

void sobel_gray_8bit(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters, sobel_operation_mode mode);

}}} // namespace cvpg::imageproc::algorithms

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_SOBEL_HPP
