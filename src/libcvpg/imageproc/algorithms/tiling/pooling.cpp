#include <libcvpg/imageproc/algorithms/tiling/pooling.hpp>

#include <algorithm>
#include <cmath>

namespace {

void pooling_gray_8bit_max(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
{
    const double scale_ratio_x = 2.0;
    const double scale_ratio_y = 2.0;

    const double inv_scale_ratio_x = 1.0 / scale_ratio_x;
    const double inv_scale_ratio_y = 1.0 / scale_ratio_y;

    const std::size_t dst_from_x = static_cast<std::size_t>(static_cast<double>(from_x) * inv_scale_ratio_x);
    const std::size_t dst_to_x = static_cast<std::size_t>((static_cast<double>(to_x) + 0.5) * inv_scale_ratio_x);
    const std::size_t dst_from_y = static_cast<std::size_t>(static_cast<double>(from_y) * inv_scale_ratio_y);
    const std::size_t dst_to_y = static_cast<std::size_t>((static_cast<double>(to_y) + 0.5) * inv_scale_ratio_y);

    const std::size_t src_image_width = parameters.image_width;
    const std::size_t dst_image_width = parameters.dst_image_width;

    std::uint8_t * src_line_y0 = nullptr;
    std::uint8_t * src_line_y1 = nullptr;
    std::uint8_t * dst_line = nullptr;

    for (std::size_t dst_y = dst_from_y; dst_y <= dst_to_y; ++dst_y)
    {
        const std::size_t src_offset_y = static_cast<std::size_t>(src_image_width * std::floor(dst_y * scale_ratio_y));
        const std::size_t dst_offset_y = dst_image_width * dst_y;

        src_line_y0 = src + src_offset_y;
        src_line_y1 = src_line_y0 + src_image_width;
        dst_line = dst + dst_offset_y;

        std::size_t src_x = from_x;

        for (std::size_t dst_x = dst_from_x; dst_x <= dst_to_x; ++dst_x)
        {
            const std::uint8_t v_y0 = std::max(src_line_y0[src_x], src_line_y0[src_x + 1]);
            const std::uint8_t v_y1 = std::max(src_line_y1[src_x], src_line_y1[src_x + 1]);

            dst_line[dst_x] = std::max(v_y0, v_y1);

            src_x += 2;
        }
    }
}

void pooling_gray_8bit_min(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
{
    const double scale_ratio_x = 2.0;
    const double scale_ratio_y = 2.0;

    const double inv_scale_ratio_x = 1.0 / scale_ratio_x;
    const double inv_scale_ratio_y = 1.0 / scale_ratio_y;

    const std::size_t dst_from_x = static_cast<std::size_t>(static_cast<double>(from_x) * inv_scale_ratio_x);
    const std::size_t dst_to_x = static_cast<std::size_t>((static_cast<double>(to_x) + 0.5) * inv_scale_ratio_x);
    const std::size_t dst_from_y = static_cast<std::size_t>(static_cast<double>(from_y) * inv_scale_ratio_y);
    const std::size_t dst_to_y = static_cast<std::size_t>((static_cast<double>(to_y) + 0.5) * inv_scale_ratio_y);

    const std::size_t src_image_width = parameters.image_width;
    const std::size_t dst_image_width = parameters.dst_image_width;

    std::uint8_t * src_line_y0 = nullptr;
    std::uint8_t * src_line_y1 = nullptr;
    std::uint8_t * dst_line = nullptr;

    for (std::size_t dst_y = dst_from_y; dst_y <= dst_to_y; ++dst_y)
    {
        const std::size_t src_offset_y = static_cast<std::size_t>(src_image_width * std::floor(dst_y * scale_ratio_y));
        const std::size_t dst_offset_y = dst_image_width * dst_y;

        src_line_y0 = src + src_offset_y;
        src_line_y1 = src_line_y0 + src_image_width;
        dst_line = dst + dst_offset_y;

        std::size_t src_x = from_x;

        for (std::size_t dst_x = dst_from_x; dst_x <= dst_to_x; ++dst_x)
        {
            const std::uint8_t v_y0 = std::min(src_line_y0[src_x], src_line_y0[src_x + 1]);
            const std::uint8_t v_y1 = std::min(src_line_y1[src_x], src_line_y1[src_x + 1]);

            dst_line[dst_x] = std::min(v_y0, v_y1);

            src_x += 2;
        }
    }
}

void pooling_gray_8bit_avg(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
{
    const double scale_ratio_x = 2.0;
    const double scale_ratio_y = 2.0;

    const double inv_scale_ratio_x = 1.0 / scale_ratio_x;
    const double inv_scale_ratio_y = 1.0 / scale_ratio_y;

    const std::size_t dst_from_x = static_cast<std::size_t>(static_cast<double>(from_x) * inv_scale_ratio_x);
    const std::size_t dst_to_x = static_cast<std::size_t>((static_cast<double>(to_x) + 0.5) * inv_scale_ratio_x);
    const std::size_t dst_from_y = static_cast<std::size_t>(static_cast<double>(from_y) * inv_scale_ratio_y);
    const std::size_t dst_to_y = static_cast<std::size_t>((static_cast<double>(to_y) + 0.5) * inv_scale_ratio_y);

    const std::size_t src_image_width = parameters.image_width;
    const std::size_t dst_image_width = parameters.dst_image_width;

    std::uint8_t * src_line_y0 = nullptr;
    std::uint8_t * src_line_y1 = nullptr;
    std::uint8_t * dst_line = nullptr;

    for (std::size_t dst_y = dst_from_y; dst_y <= dst_to_y; ++dst_y)
    {
        const std::size_t src_offset_y = static_cast<std::size_t>(src_image_width * std::floor(dst_y * scale_ratio_y));
        const std::size_t dst_offset_y = dst_image_width * dst_y;

        src_line_y0 = src + src_offset_y;
        src_line_y1 = src_line_y0 + src_image_width;
        dst_line = dst + dst_offset_y;

        std::size_t src_x = from_x;

        for (std::size_t dst_x = dst_from_x; dst_x <= dst_to_x; ++dst_x)
        {
            std::uint16_t v = static_cast<std::uint16_t>(src_line_y0[src_x]);
            v += static_cast<std::uint16_t>(src_line_y0[src_x + 1]);
            v += static_cast<std::uint16_t>(src_line_y1[src_x]);
            v += static_cast<std::uint16_t>(src_line_y1[src_x + 1]);
            v = v >> 2; // divide by 4

            dst_line[dst_x] = static_cast<std::uint8_t>(v);

            src_x += 2;
        }
    }
}

}

namespace cvpg::imageproc::algorithms {

void pooling_gray_8bit(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters, pooling_operation_mode mode)
{
    if (mode == pooling_operation_mode::max)
    {
        pooling_gray_8bit_max(src, dst, from_x, to_x, from_y, to_y, std::move(parameters));
    }
    else if (mode == pooling_operation_mode::min)
    {
        pooling_gray_8bit_min(src, dst, from_x, to_x, from_y, to_y, std::move(parameters));
    }
    else if (mode == pooling_operation_mode::avg)
    {
        pooling_gray_8bit_avg(src, dst, from_x, to_x, from_y, to_y, std::move(parameters));
    }
}

} // namespace cvpg::imageproc::algorithms
