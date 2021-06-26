// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_HISTOGRAM_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_HISTOGRAM_HPP

#include <cstdint>
#include <vector>

#include <libcvpg/imageproc/algorithms/tiling/parameters.hpp>

namespace cvpg { namespace imageproc { namespace algorithms {

void histogram_gray_8bit(std::uint8_t * src, std::vector<std::size_t> * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters);

}}} // namespace cvpg::imageproc::algorithms

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_HISTOGRAM_HPP
