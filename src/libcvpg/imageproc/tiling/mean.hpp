#ifndef LIBCVPG_IMAGEPROC_TILING_MEAN_HPP
#define LIBCVPG_IMAGEPROC_TILING_MEAN_HPP

#include <cstdint>

#include <libcvpg/imageproc/tiling.hpp>

namespace cvpg { namespace imageproc {

void mean_gray_8bit(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::tiling_params params);

}} // namespace cvpg::imageproc

#endif // LIBCVPG_IMAGEPROC_TILING_MEAN_HPP
