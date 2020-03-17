#include <libcvpg/imageproc/algorithms/tiling/mean.hpp>

#include <cstring>
#include <deque>

namespace cvpg { namespace imageproc { namespace algorithms {

void mean_gray_8bit(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
{
    const std::size_t org_from_x = from_x;
    const std::size_t org_to_x = to_x;

    const std::size_t org_from_y = from_y;
    const std::size_t org_to_y = to_y;

    const std::size_t image_width = parameters.image_width;
    const std::size_t image_height = parameters.image_height;

    const std::uint32_t filter_width = parameters.signed_integer_numbers.at(0);
    const std::uint32_t filter_height = parameters.signed_integer_numbers.at(1);

    const std::uint32_t half_filter_width = filter_width / 2;
    const std::uint32_t half_filter_height = filter_height / 2;

    std::deque<std::int32_t> vert;

    const double filter_size_ = 1.0 / (filter_width * filter_height);

    std::uint8_t * src_line = nullptr;
    std::uint8_t * dst_line = nullptr;

    // fix from/to values only at image border
    if (from_x == 0)
    {
        from_x += half_filter_width;
    }

    if (to_x == image_width - 1)
    {
        to_x -= half_filter_width;
    }

    if (from_y == 0)
    {
        from_y += half_filter_height;
    }

    if (to_y == image_height - 1)
    {
        to_y -= half_filter_height;
    }

    for (std::int32_t y = from_y; y <= to_y; ++y)
    {
        const std::size_t offset_y = image_width * y;

        dst_line = dst + offset_y;

        vert.clear();

        // fill vector of vertical sums
        for (std::size_t fx = from_x - half_filter_width; fx <= from_x + half_filter_width; ++fx)
        {
            std::int32_t sum = 0;

            for (std::size_t fy = y - half_filter_height; fy <= y + half_filter_height; ++fy)
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

        for (std::size_t x = from_x; x <= to_x; ++x)
        {
            std::int32_t sum = 0;

            for (std::size_t fy = y - half_filter_height; fy <= y + half_filter_height; ++fy)
            {
                sum += static_cast<std::int32_t>((src + image_width * fy)[x + half_filter_width]);
            }

            hsum += sum;
            hsum -= vert.front();

            vert.push_back(sum);
            vert.pop_front();

            dst_line[x] = static_cast<std::uint8_t>(hsum * filter_size_);
        }
    }

    if (org_from_x == 0)
    {
        for (std::size_t y = org_from_y; y <= org_to_y; ++y)
        {
            memcpy(dst + y * image_width, dst + y * image_width + half_filter_width + 1, half_filter_width);
        }
    }

    if (org_to_x == image_width - 1)
    {
        for (std::size_t y = org_from_y; y <= org_to_y; ++y)
        {
            memcpy(dst + (y + 1) * image_width - half_filter_width, dst + (y + 1) * image_width - filter_width - 1, half_filter_width);
        }
    }

    if (org_from_y == 0)
    {
        for (std::size_t y = 0; y < half_filter_height; ++y)
        {
            memcpy(dst + y * image_width, dst + half_filter_height * image_width, image_width);
        }
    }

    if (org_to_y == image_height - 1)
    {
        for (std::size_t y = image_height - half_filter_height; y < image_height; ++y)
        {
            memcpy(dst + y * image_width, dst + (image_height - half_filter_height - 1) * image_width, image_width);
        }
    }
}

}}} // namespace cvpg::imageproc::algorithms
