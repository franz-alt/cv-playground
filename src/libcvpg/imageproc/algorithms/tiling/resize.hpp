// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_RESIZE_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_RESIZE_HPP

#include <cstdint>

#include <libcvpg/imageproc/algorithms/tiling/parameters.hpp>

namespace cvpg::imageproc::algorithms {

void resize_gray_8bit(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters);

} // namespace cvpg::imageproc::algorithms

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_RESIZE_HPP
