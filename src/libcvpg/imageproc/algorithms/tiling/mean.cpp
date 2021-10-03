// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/imageproc/algorithms/tiling/mean.hpp>

#include <deque>

namespace {

void mean_gray_8bit_kernel_ignore_border(std::uint8_t * src, std::uint8_t * dst, std::int32_t from_x, std::int32_t to_x, std::int32_t from_y, std::int32_t to_y, std::int32_t image_width, std::int32_t filter_width, std::int32_t filter_height)
{
    std::uint8_t * src_line = nullptr;
    std::uint8_t * dst_line = nullptr;

    const double inv_filter_size = 1.0 / (filter_width * filter_height);

    const std::int32_t half_filter_width = filter_width >> 1;
    const std::int32_t half_filter_height = filter_height >> 1;

    std::deque<std::int32_t> vert;

    for (std::int32_t y = from_y; y < to_y; ++y)
    {
        const std::int32_t offset_y = image_width * y;

        dst_line = dst + offset_y;

        vert.clear();

        // fill vector of vertical sums
        for (std::int32_t fx = from_x - half_filter_width; fx <= from_x + half_filter_width; ++fx)
        {
            std::int32_t sum = 0;

            for (std::int32_t fy = y - half_filter_height; fy <= y + half_filter_height; ++fy)
            {
                sum += static_cast<std::int32_t>((src + image_width * fy)[fx]);
            }

            vert.push_back(sum);
        }

        std::int32_t hsum = 0;

        for (std::size_t i = 0; i < vert.size(); ++i)
        {
            hsum += vert[i];
        }

        for (std::int32_t x = from_x; x < to_x; ++x)
        {
            std::int32_t sum = 0;

            for (std::int32_t fy = y - half_filter_height; fy <= y + half_filter_height; ++fy)
            {
                sum += static_cast<std::int32_t>((src + image_width * fy)[x + half_filter_width]);
            }

            hsum += sum;
            hsum -= vert.front();

            vert.push_back(sum);
            vert.pop_front();

            dst_line[x] = static_cast<std::uint8_t>(hsum * inv_filter_size);
        }
    }
}

void mean_gray_8bit_kernel_constant_border(std::uint8_t * src, std::uint8_t * dst, std::int32_t from_x, std::int32_t to_x, std::int32_t from_y, std::int32_t to_y, std::int32_t image_width, std::int32_t image_height, std::int32_t filter_width, std::int32_t filter_height)
{
    std::uint8_t * src_line = nullptr;
    std::uint8_t * dst_line = nullptr;

    const double inv_filter_size = 1.0 / (filter_width * filter_height);

    const std::int32_t half_filter_width = filter_width >> 1;
    const std::int32_t half_filter_height = filter_height >> 1;

    std::deque<std::int32_t> vert;

    for (std::int32_t y = from_y; y <= to_y; ++y)
    {
        const std::int32_t offset_y = image_width * y;

        dst_line = dst + offset_y;

        vert.clear();

        // fill vector of vertical sums
        for (std::int32_t fx = from_x - half_filter_width; fx <= from_x + half_filter_width; ++fx)
        {
            std::int32_t sum = 0;

            for (std::int32_t fy = y - half_filter_height; fy <= y + half_filter_height; ++fy)
            {
                sum += (fy >= 0 && fy < image_height && fx >= 0 && fx < image_width) ? static_cast<std::int32_t>((src + image_width * fy)[fx]) : 0;
            }

            vert.push_back(sum);
        }

        std::int32_t hsum = 0;

        for (std::size_t i = 0; i < vert.size(); ++i)
        {
            hsum += vert[i];
        }

        for (std::int32_t x = from_x; x <= to_x; ++x)
        {
            std::int32_t sum = 0;
            std::int32_t x_ = x + half_filter_width;

            for (std::int32_t fy = y - half_filter_height; fy <= y + half_filter_height; ++fy)
            {
                sum += (fy >= 0 && fy < image_height && x_ >= 0 && x_ < image_width) ? static_cast<std::int32_t>((src + image_width * fy)[x_]) : 0;
            }

            hsum += sum;
            hsum -= vert.front();

            vert.push_back(sum);
            vert.pop_front();

            dst_line[x] = static_cast<std::uint8_t>(hsum * inv_filter_size);
        }
    }
}

void mean_gray_8bit_kernel_mirror_border(std::uint8_t * src, std::uint8_t * dst, std::int32_t from_x, std::int32_t to_x, std::int32_t from_y, std::int32_t to_y, std::int32_t image_width, std::int32_t image_height, std::int32_t filter_width, std::int32_t filter_height)
{
    std::uint8_t * src_line = nullptr;
    std::uint8_t * dst_line = nullptr;

    const double inv_filter_size = 1.0 / (filter_width * filter_height);

    const std::int32_t half_filter_width = filter_width >> 1;
    const std::int32_t half_filter_height = filter_height >> 1;

    std::deque<std::int32_t> vert;

    for (std::int32_t y = from_y; y < to_y; ++y)
    {
        const std::size_t offset_y = image_width * y;

        dst_line = dst + offset_y;

        vert.clear();

        // fill vector of vertical sums
        for (std::int32_t fx = from_x - half_filter_width; fx <= from_x + half_filter_width; ++fx)
        {
            std::int32_t sum = 0;
            std::int32_t fx_ = fx >= image_width ? (fx - image_width) : (fx >= 0 ? fx : (image_width + fx));
            std::int32_t fy_ = 0;

            for (std::int32_t fy = y - half_filter_height; fy <= y + half_filter_height; ++fy)
            {
                fy_ = fy >= image_height ? (fy - image_height) : (fy >= 0 ? fy : (image_height + fy));

                sum += static_cast<std::int32_t>((src + image_width * fy_)[fx_]);
            }

            vert.push_back(sum);
        }

        std::int32_t hsum = 0;

        for (std::size_t i = 0; i < vert.size(); ++i)
        {
            hsum += vert[i];
        }

        for (std::int32_t x = from_x; x < to_x; ++x)
        {
            std::int32_t sum = 0;
            std::int32_t x_  = x + half_filter_width;
            std::int32_t x__ = x_ >= image_width ? (x_ - image_width) : (x_ >= 0 ? x_ : (image_width + x_));
            std::int32_t fy_ = 0;

            for (std::int32_t fy = y - half_filter_height; fy <= y + half_filter_height; ++fy)
            {
                fy_ = fy >= image_height ? (fy - image_height) : (fy >= 0 ? fy : (image_height + fy));

                sum += static_cast<std::int32_t>((src + image_width * fy_)[x__]);
            }

            hsum += sum;
            hsum -= vert.front();

            vert.push_back(sum);
            vert.pop_front();

            dst_line[x] = static_cast<std::uint8_t>(hsum * inv_filter_size);
        }
    }
}

}

namespace cvpg::imageproc::algorithms {

void mean_gray_8bit(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
{
    std::int32_t from_x_ = static_cast<std::int32_t>(from_x);
    std::int32_t to_x_ = static_cast<std::int32_t>(to_x);
    std::int32_t from_y_ = static_cast<std::int32_t>(from_y);
    std::int32_t to_y_ = static_cast<std::int32_t>(to_y);

    const std::int32_t image_width = static_cast<std::int32_t>(parameters.image_width);
    const std::int32_t image_height = static_cast<std::int32_t>(parameters.image_height);

    const std::int32_t filter_width = static_cast<std::int32_t>(parameters.signed_integer_numbers.at(0));
    const std::int32_t filter_height = static_cast<std::int32_t>(parameters.signed_integer_numbers.at(1));

    const std::int32_t half_filter_width = filter_width >> 1;
    const std::int32_t half_filter_height = filter_height >> 1;

    if (from_x_ <= half_filter_width)
    {
        from_x_ = parameters.border_mode == cvpg::imageproc::algorithms::border_mode::ignore ? half_filter_width : 0;
    }
    else
    {
        from_x_ -= half_filter_width;
    }

    if (to_x_ >= (image_width - half_filter_width))
    {
        to_x_ = image_width - (parameters.border_mode == cvpg::imageproc::algorithms::border_mode::ignore ? half_filter_width : 0);
    }
    else
    {
        to_x_ += half_filter_width;
    }

    if (from_y_ <= half_filter_height)
    {
        from_y_ = parameters.border_mode == cvpg::imageproc::algorithms::border_mode::ignore ? half_filter_height : 0;
    }
    else
    {
        from_y_ -= half_filter_height;
    }

    if (to_y_ >= (image_height - half_filter_height))
    {
        to_y_ = image_height - (parameters.border_mode == cvpg::imageproc::algorithms::border_mode::ignore ? half_filter_height : 0);
    }
    else
    {
        to_y_ += half_filter_height;
    }

    if (parameters.border_mode == cvpg::imageproc::algorithms::border_mode::ignore)
    {
        mean_gray_8bit_kernel_ignore_border(src, dst, from_x_, to_x_, from_y_, to_y_, image_width, filter_width, filter_height);
    }
    else if (parameters.border_mode == cvpg::imageproc::algorithms::border_mode::constant)
    {
        mean_gray_8bit_kernel_constant_border(src, dst, from_x, to_x, from_y, to_y, image_width, image_height, filter_width, filter_height);
    }
    else if (parameters.border_mode == cvpg::imageproc::algorithms::border_mode::mirror)
    {
        mean_gray_8bit_kernel_mirror_border(src, dst, from_x_, to_x_, from_y_, to_y_, image_width, image_height, filter_width, filter_height);
    }
}

} // namespace cvpg::imageproc::algorithms
