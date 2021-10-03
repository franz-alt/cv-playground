// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/imageproc/algorithms/tiling/resize.hpp>

#include <cmath>

namespace cvpg::imageproc::algorithms {

void resize_gray_8bit(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
{
    const double scale_ratio_x = static_cast<double>(parameters.image_width) / static_cast<double>(parameters.dst_image_width);
    const double scale_ratio_y = static_cast<double>(parameters.image_height) / static_cast<double>(parameters.dst_image_height);

    const double inv_scale_ratio_x = 1.0 / scale_ratio_x;
    const double inv_scale_ratio_y = 1.0 / scale_ratio_y;

    const std::size_t dst_from_x = static_cast<std::size_t>(static_cast<double>(from_x) * inv_scale_ratio_x);
    const std::size_t dst_to_x = static_cast<std::size_t>((static_cast<double>(to_x) + 0.5) * inv_scale_ratio_x);
    const std::size_t dst_from_y = static_cast<std::size_t>(static_cast<double>(from_y) * inv_scale_ratio_y);
    const std::size_t dst_to_y = static_cast<std::size_t>((static_cast<double>(to_y) + 0.5) * inv_scale_ratio_y);

    const std::size_t src_image_width = parameters.image_width;
    const std::size_t dst_image_width = parameters.dst_image_width;

    std::uint8_t * src_line = nullptr;
    std::uint8_t * dst_line = nullptr;

    for (std::size_t dst_y = dst_from_y; dst_y <= dst_to_y; ++dst_y)
    {
        const std::size_t src_offset_y = static_cast<std::size_t>(src_image_width * std::floor(dst_y * scale_ratio_y));
        const std::size_t dst_offset_y = dst_image_width * dst_y;

        src_line = src + src_offset_y;
        dst_line = dst + dst_offset_y;

        for (std::size_t dst_x = dst_from_x; dst_x <= dst_to_x; ++dst_x)
        {
            dst_line[dst_x] = src_line[static_cast<std::size_t>(std::floor(dst_x * scale_ratio_x))];
        }
    }   
}

} // namespace cvpg::imageproc::algorithms
