// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/imageproc/algorithms/tiling/threshold.hpp>

#include <algorithm>

namespace cvpg { namespace imageproc { namespace algorithms {

void threshold_gray_8bit(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
{
    const std::size_t image_width = parameters.image_width;

    const std::int32_t threshold = parameters.signed_integer_numbers.at(0);

    std::uint8_t * src_line = nullptr;
    std::uint8_t * dst_line = nullptr;

    for (std::size_t y = from_y; y <= to_y; ++y)
    {
        const std::size_t offset_y = image_width * y;

        src_line = src + offset_y;
        dst_line = dst + offset_y;

        for (std::size_t x = from_x; x <= to_x; ++x)
        {
            dst_line[x] = (src_line[x] >= threshold) ? 255 : 0;
        }
    }
}

void threshold_inverse_gray_8bit(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
{
    const std::size_t image_width = parameters.image_width;

    const std::int32_t threshold = parameters.signed_integer_numbers.at(0);

    std::uint8_t * src_line = nullptr;
    std::uint8_t * dst_line = nullptr;

    for (std::size_t y = from_y; y <= to_y; ++y)
    {
        const std::size_t offset_y = image_width * y;

        src_line = src + offset_y;
        dst_line = dst + offset_y;

        for (std::size_t x = from_x; x <= to_x; ++x)
        {
            dst_line[x] = (src_line[x] >= threshold) ? 0 : 255;
        }
    }
}

}}} // namespace cvpg::imageproc::algorithms
