#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_MEAN_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_MEAN_HPP

#include <cstdint>

#include <libcvpg/imageproc/algorithms/tiling.hpp>

namespace cvpg { namespace imageproc { namespace algorithms {

void mean_gray_8bit(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_params params);

}}} // namespace cvpg::imageproc::algorithms

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_MEAN_HPP
