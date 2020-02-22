#include <libcvpg/imageproc/tiling/diff.hpp>

namespace cvpg { namespace imageproc {

void diff_gray_8bit(std::uint8_t * src1, std::uint8_t * src2, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::tiling_params params)
{
    const std::size_t image_width = params.image_width;

    const std::int32_t offset = params.offset;

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
            std::int16_t v = static_cast<std::int16_t>(src1_line[x]) - static_cast<std::int16_t>(src2_line[x]) + static_cast<std::int16_t>(offset);

            dst_line[x] = static_cast<std::uint8_t>(std::max(static_cast<std::int16_t>(0), std::min(static_cast<std::int16_t>(255), v)));
        }
    }
}

}} // namespace cvpg::imageproc
