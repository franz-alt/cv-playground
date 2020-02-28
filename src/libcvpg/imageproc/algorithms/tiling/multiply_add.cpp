#include <libcvpg/imageproc/algorithms/tiling/multiply_add.hpp>

namespace cvpg { namespace imageproc { namespace algorithms {

void multiply_add_gray_8bit(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_params params)
{
    const std::size_t image_width = params.image_width;

    const double factor = params.factor;
    const std::int32_t offset = params.offset;

    std::uint8_t * src_line = nullptr;
    std::uint8_t * dst_line = nullptr;

    for (std::size_t y = from_y; y <= to_y; ++y)
    {
        const std::size_t offset_y = image_width * y;

        src_line = src + offset_y;
        dst_line = dst + offset_y;

        for (std::size_t x = from_x; x <= to_x; ++x)
        {
            std::int16_t v = static_cast<std::int16_t>(src_line[x] * factor) + static_cast<std::int16_t>(offset);

            dst_line[x] = static_cast<std::uint8_t>(std::max(static_cast<std::int16_t>(0), std::min(static_cast<std::int16_t>(255), v)));
        }
    }
}

}}} // namespace cvpg::imageproc::algorithms
