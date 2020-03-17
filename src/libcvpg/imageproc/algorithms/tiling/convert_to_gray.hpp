#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_CONVERT_TO_GRAY_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_CONVERT_TO_GRAY_HPP

#include <cstdint>

#include <libcvpg/imageproc/algorithms/tiling/parameters.hpp>

namespace cvpg { namespace imageproc { namespace algorithms {

void convert_to_gray_8bit(std::uint8_t * src1, std::uint8_t * src2, std::uint8_t * src3, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters);

}}} // namespace cvpg::imageproc::algorithms

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_CONVERT_TO_GRAY_HPP
