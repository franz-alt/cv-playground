// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/imageproc/algorithms/tiling/histogram.hpp>

namespace cvpg { namespace imageproc { namespace algorithms {

void histogram_gray_8bit(std::uint8_t * src, std::vector<std::size_t> * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
{
    const std::size_t image_width = parameters.image_width;

    std::uint8_t * src_line = nullptr;

    for (std::size_t y = from_y; y <= to_y; ++y)
    {
        src_line = src + image_width * y;

        for (std::size_t x = from_x; x <= to_x; ++x)
        {
            ++((*dst)[src_line[x]]);
        }
    }
}

}}} // namespace cvpg::imageproc::algorithms
