// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/imageproc/algorithms/tiling/and.hpp>

namespace cvpg::imageproc::algorithms {

void and_gray_8bit(std::uint8_t * src1, std::uint8_t * src2, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
{
    const std::size_t image_width = parameters.image_width;

    std::uint8_t * src1_line = nullptr;
    std::uint8_t * src2_line = nullptr;
    std::uint8_t * dst_line  = nullptr;

    for (std::size_t y = from_y; y <= to_y; ++y)
    {
        const std::size_t offset_y = image_width * y;

        src1_line = src1 + offset_y;
        src2_line = src2 + offset_y;
        dst_line  = dst  + offset_y;

        for (std::size_t x = from_x; x <= to_x; ++x)
        {
            dst_line[x] = src1_line[x] & src2_line[x];
        }
    }
}

} // namespace cvpg::imageproc::algorithms
