#include <libcvpg/imageproc/algorithms/tiling/scharr.hpp>

namespace {

void scharr_gray_8bit_kernel_3x3_ignore_border(std::uint8_t * src, std::uint8_t * dst, std::int32_t from_x, std::int32_t to_x, std::int32_t from_y, std::int32_t to_y, std::int32_t image_width, cvpg::imageproc::algorithms::scharr_operation_mode mode)
{
    std::uint8_t * src_line_m1 = nullptr;  // begin of previous line in source image
    std::uint8_t * src_line    = nullptr;  // begin of current line in source image
    std::uint8_t * src_line_p1 = nullptr;  // begin of next line in source image

    std::uint8_t * dst_line = nullptr;

    if (mode == cvpg::imageproc::algorithms::scharr_operation_mode::horizontal)
    {
        //
        // perform D = Gx * S
        //
        //      | g00 g01 g02 |   |  +3  0  -3 |
        // Gx = | g10 g11 g12 | = | +10  0 -10 |
        //      | g20 g21 g22 |   |  +3  0  -3 |
        //
        //      | s00 s01 s02 |
        // S  = | s10 s11 s12 | ; a11 = current image pixel
        //      | s20 s21 s22 |
        //

        for (std::int32_t y = from_y; y < to_y; ++y)
        {
            const std::int32_t offset_ym1 = image_width * (y - 1);   // offset previous line
            const std::int32_t offset_y0  = image_width * y;         // offset current line
            const std::int32_t offset_yp1 = image_width * (y + 1);   // offset next line

            src_line_m1 = src + offset_ym1;
            src_line    = src + offset_y0;
            src_line_p1 = src + offset_yp1;

            dst_line = dst + offset_y0;

            for (std::int32_t x = from_x; x < to_x; ++x)
            {
                const std::int16_t d00 = src_line_m1[x - 1] * 3;
                const std::int16_t d02 = src_line_m1[x + 1] * 3;
                const std::int16_t d10 = src_line[x - 1] * 10;
                const std::int16_t d12 = src_line[x + 1] * 10;
                const std::int16_t d20 = src_line_p1[x - 1] * 3;
                const std::int16_t d22 = src_line_p1[x + 1] * 3;

                const std::int16_t res = d00 - d02 + d10 - d12 + d20 - d22;

                if (res > 255)
                {
                    dst_line[x] = 255;
                }
                else if (res < 0)
                {
                    dst_line[x] = 0;
                }
                else
                {
                    dst_line[x] = static_cast<std::uint8_t>(res);
                }
            }
        }
    }
    else if (mode == cvpg::imageproc::algorithms::scharr_operation_mode::vertical)
    {
        //
        // perform D = Gy * S
        //
        //      | g00 g01 g02 |   |  +3 +10  +3 |
        // Gy = | g10 g11 g12 | = |   0   0   0 |
        //      | g20 g21 g22 |   |  -3 -10  -3 |
        //
        //      | s00 s01 s02 |
        // S  = | s10 s11 s12 | ; a11 = current image pixel
        //      | s20 s21 s22 |
        //

        for (std::int32_t y = from_y; y < to_y; ++y)
        {
            const std::int32_t offset_ym1 = image_width * (y - 1);   // offset previous line
            const std::int32_t offset_y0  = image_width * y;         // offset current line
            const std::int32_t offset_yp1 = image_width * (y + 1);   // offset next line

            src_line_m1 = src + offset_ym1;
            src_line    = src + offset_y0;
            src_line_p1 = src + offset_yp1;

            dst_line = dst + offset_y0;

            for (std::int32_t x = from_x; x < to_x; ++x)
            {
                const std::int16_t d00 = src_line_m1[x - 1] * 3;
                const std::int16_t d01 = src_line_m1[x] * 10;
                const std::int16_t d02 = src_line_m1[x + 1] * 3;
                const std::int16_t d20 = src_line_p1[x - 1] * 3;
                const std::int16_t d21 = src_line_p1[x] * 10;
                const std::int16_t d22 = src_line_p1[x + 1] * 3;

                const std::int16_t res = d00 + d01 + d02 - d20 - d21 - d22;

                if (res > 255)
                {
                    dst_line[x] = 255;
                }
                else if (res < 0)
                {
                    dst_line[x] = 0;
                }
                else
                {
                    dst_line[x] = static_cast<std::uint8_t>(res);
                }
            }
        }
    }
}

void scharr_gray_8bit_kernel_3x3_constant_border(std::uint8_t * src, std::uint8_t * dst, std::int32_t from_x, std::int32_t to_x, std::int32_t from_y, std::int32_t to_y, std::int32_t image_width, std::int32_t image_height, cvpg::imageproc::algorithms::scharr_operation_mode mode)
{
    std::uint8_t * src_line_m1 = nullptr;  // begin of previous line in source image
    std::uint8_t * src_line    = nullptr;  // begin of current line in source image
    std::uint8_t * src_line_p1 = nullptr;  // begin of next line in source image

    std::uint8_t * dst_line = nullptr;

    if (mode == cvpg::imageproc::algorithms::scharr_operation_mode::horizontal)
    {
        //
        // perform D = Gx * S
        //
        //      | g00 g01 g02 |   |  +3   0  -3 |
        // Gx = | g10 g11 g12 | = | +10   0 -10 |
        //      | g20 g21 g22 |   |  +3   0  -3 |
        //
        //      | s00 s01 s02 |
        // S  = | s10 s11 s12 | ; a11 = current image pixel
        //      | s20 s21 s22 |
        //

        for (std::int32_t y = from_y; y < to_y; ++y)
        {
            const std::int32_t offset_ym1 = image_width * (y - 1);   // offset previous line
            const std::int32_t offset_y0  = image_width * y;         // offset current line
            const std::int32_t offset_yp1 = image_width * (y + 1);   // offset next line

            src_line_m1 = y != 0 ? (src + offset_ym1) : nullptr;
            src_line    = src + offset_y0;
            src_line_p1 = y != (image_height - 1) ? (src + offset_yp1) : nullptr;

            dst_line = dst + offset_y0;

            for (std::int32_t x = from_x; x < to_x; ++x)
            {
                const std::int16_t d00 = src_line_m1 != nullptr ? (x != 0 ? (src_line_m1[x - 1] * 3) : 0) : 0;
                const std::int16_t d02 = src_line_m1 != nullptr ? (x != (image_width - 1) ? (src_line_m1[x + 1] * 3) : 0) : 0;
                const std::int16_t d10 = src_line[x - 1] * 10;
                const std::int16_t d12 = src_line[x + 1] * 10;
                const std::int16_t d20 = src_line_p1 != nullptr ? (x != 0 ? (src_line_p1[x - 1] * 3) : 0) : 0;
                const std::int16_t d22 = src_line_p1 != nullptr ? (x != (image_width - 1) ? (src_line_p1[x + 1] * 3) : 0) : 0;

                const std::int16_t res = d00 - d02 + d10 - d12 + d20 - d22;

                if (res > 255)
                {
                    dst_line[x] = 255;
                }
                else if (res < 0)
                {
                    dst_line[x] = 0;
                }
                else
                {
                    dst_line[x] = static_cast<std::uint8_t>(res);
                }
            }
        }
    }
    else if (mode == cvpg::imageproc::algorithms::scharr_operation_mode::vertical)
    {
        //
        // perform D = Gy * S
        //
        //      | g00 g01 g02 |   |  +3 +10  +3 |
        // Gy = | g10 g11 g12 | = |   0   0   0 |
        //      | g20 g21 g22 |   |  -3 -10  -3 |
        //
        //      | s00 s01 s02 |
        // S  = | s10 s11 s12 | ; a11 = current image pixel
        //      | s20 s21 s22 |
        //

        for (std::int32_t y = from_y; y < to_y; ++y)
        {
            const std::int32_t offset_ym1 = image_width * (y - 1);   // offset previous line
            const std::int32_t offset_y0  = image_width * y;         // offset current line
            const std::int32_t offset_yp1 = image_width * (y + 1);   // offset next line

            src_line_m1 = y != 0 ? (src + offset_ym1) : nullptr;
            src_line    = src + offset_y0;
            src_line_p1 = y != (image_height - 1) ? (src + offset_yp1) : nullptr;

            dst_line = dst + offset_y0;

            for (std::int32_t x = from_x; x < to_x; ++x)
            {
                const std::int16_t d00 = src_line_m1 != nullptr ? (x != 0 ? (src_line_m1[x - 1] * 3) : 0) : 0;
                const std::int16_t d01 = src_line_m1 != nullptr ? (src_line_m1[x] * 10) : 0;
                const std::int16_t d02 = src_line_m1 != nullptr ? (x != (image_width - 1) ? (src_line_m1[x + 1] * 3) : 0) : 0;
                const std::int16_t d20 = src_line_p1 != nullptr ? (x != 0 ? (src_line_p1[x - 1] * 3) : 0) : 0;
                const std::int16_t d21 = src_line_p1 != nullptr ? (src_line_p1[x] * 10) : 0;
                const std::int16_t d22 = src_line_p1 != nullptr ? (x != (image_width - 1) ? (src_line_p1[x + 1] * 3) : 0) : 0;

                const std::int16_t res = d00 + d01 + d02 - d20 - d21 - d22;

                if (res > 255)
                {
                    dst_line[x] = 255;
                }
                else if (res < 0)
                {
                    dst_line[x] = 0;
                }
                else
                {
                    dst_line[x] = static_cast<std::uint8_t>(res);
                }
            }
        }
    }
}

void scharr_gray_8bit_kernel_3x3_mirror_border(std::uint8_t * src, std::uint8_t * dst, std::int32_t from_x, std::int32_t to_x, std::int32_t from_y, std::int32_t to_y, std::int32_t image_width, std::int32_t image_height, cvpg::imageproc::algorithms::scharr_operation_mode mode)
{
    std::uint8_t * src_line_m1 = nullptr;  // begin of previous line in source image
    std::uint8_t * src_line    = nullptr;  // begin of current line in source image
    std::uint8_t * src_line_p1 = nullptr;  // begin of next line in source image

    std::uint8_t * dst_line = nullptr;

    if (mode == cvpg::imageproc::algorithms::scharr_operation_mode::horizontal)
    {
        //
        // perform D = Gx * S
        //
        //      | g00 g01 g02 |   |  +3   0  -3 |
        // Gx = | g10 g11 g12 | = | +10   0 -10 |
        //      | g20 g21 g22 |   |  +3   0  -3 |
        //
        //      | s00 s01 s02 |
        // S  = | s10 s11 s12 | ; a11 = current image pixel
        //      | s20 s21 s22 |
        //

        for (std::int32_t y = from_y; y < to_y; ++y)
        {
            const std::int32_t offset_ym1 = y != 0 ? (image_width * (y - 1)) : (image_width * (image_height - 1));     // offset previous line
            const std::int32_t offset_y0  = image_width * y;                                                             // offset current line
            const std::int32_t offset_yp1 = y != (image_height - 1) ? (image_width * (y + 1)) : 0;                     // offset next line

            src_line_m1 = src + offset_ym1;
            src_line    = src + offset_y0;
            src_line_p1 = src + offset_yp1;

            dst_line = dst + offset_y0;

            for (std::int32_t x = from_x; x < to_x; ++x)
            {
                const std::int16_t d00 = src_line_m1[x != 0 ? (x - 1) : (image_width - 1)] * 3;
                const std::int16_t d02 = src_line_m1[x != (image_width - 1) ? (x + 1) : 0] * 3;
                const std::int16_t d10 = src_line[x != 0 ? (x - 1) : (image_width - 1)] * 10;
                const std::int16_t d12 = src_line[x != (image_width - 1) ? (x + 1) : 0] * 10;
                const std::int16_t d20 = src_line_p1[x != 0 ? (x - 1) : (image_width - 1)] * 3;
                const std::int16_t d22 = src_line_p1[x != (image_width - 1) ? (x + 1) : 0] * 3;

                const std::int16_t res = d00 - d02 + d10 - d12 + d20 - d22;

                if (res > 255)
                {
                    dst_line[x] = 255;
                }
                else if (res < 0)
                {
                    dst_line[x] = 0;
                }
                else
                {
                    dst_line[x] = static_cast<std::uint8_t>(res);
                }
            }
        }
    }
    else if (mode == cvpg::imageproc::algorithms::scharr_operation_mode::vertical)
    {
        //
        // perform D = Gy * S
        //
        //      | g00 g01 g02 |   |  +3 +10  +3 |
        // Gy = | g10 g11 g12 | = |   0   0   0 |
        //      | g20 g21 g22 |   |  -3 -10  -3 |
        //
        //      | s00 s01 s02 |
        // S  = | s10 s11 s12 | ; a11 = current image pixel
        //      | s20 s21 s22 |
        //

        for (std::int32_t y = from_y; y < to_y; ++y)
        {
            const std::int32_t offset_ym1 = y != 0 ? (image_width * (y - 1)) : (image_width * (image_height - 1));     // offset previous line
            const std::int32_t offset_y0  = image_width * y;                                                             // offset current line
            const std::int32_t offset_yp1 = y != (image_height - 1) ? (image_width * (y + 1)) : 0;                     // offset next line

            src_line_m1 = src + offset_ym1;
            src_line    = src + offset_y0;
            src_line_p1 = src + offset_yp1;

            dst_line = dst + offset_y0;

            for (std::int32_t x = from_x; x < to_x; ++x)
            {
                const std::int16_t d00 = src_line_m1[x != 0 ? (x - 1) : (image_width - 1)] * 3;
                const std::int16_t d01 = src_line_m1[x] * 10;
                const std::int16_t d02 = src_line_m1[x != (image_width - 1) ? (x + 1) : 0] * 3;
                const std::int16_t d20 = src_line_p1[x != 0 ? (x - 1) : (image_width - 1)] * 3;
                const std::int16_t d21 = src_line_p1[x] * 10;
                const std::int16_t d22 = src_line_p1[x != (image_width - 1) ? (x + 1) : 0] * 3;

                const std::int16_t res = d00 + d01 + d02 - d20 - d21 - d22;

                if (res > 255)
                {
                    dst_line[x] = 255;
                }
                else if (res < 0)
                {
                    dst_line[x] = 0;
                }
                else
                {
                    dst_line[x] = static_cast<std::uint8_t>(res);
                }
            }
        }
    }
}

}

namespace cvpg { namespace imageproc { namespace algorithms {

void scharr_gray_8bit(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters, scharr_operation_mode mode)
{
    std::int32_t from_x_ = static_cast<std::int32_t>(from_x);
    std::int32_t to_x_ = static_cast<std::int32_t>(to_x);
    std::int32_t from_y_ = static_cast<std::int32_t>(from_y);
    std::int32_t to_y_ = static_cast<std::int32_t>(to_y);

    const std::int32_t image_width = static_cast<std::int32_t>(parameters.image_width);
    const std::int32_t image_height = static_cast<std::int32_t>(parameters.image_height);

    const std::int32_t size = parameters.signed_integer_numbers.at(0);

    // correct from/to values ; they have to overlap to make scharr work correctly
    const std::int32_t half_size = size >> 1;

    if (from_x_ <= half_size)
    {
        from_x_ = parameters.border_mode == cvpg::imageproc::algorithms::border_mode::ignore ? half_size : 0;
    }
    else
    {
        from_x_ -= half_size;
    }

    if (to_x_ >= (image_width - half_size))
    {
        to_x_ = image_width - (parameters.border_mode == cvpg::imageproc::algorithms::border_mode::ignore ? half_size : 0);
    }
    else
    {
        to_x_ += half_size;
    }

    if (from_y_ <= half_size)
    {
        from_y_ = parameters.border_mode == cvpg::imageproc::algorithms::border_mode::ignore ? half_size : 0;
    }
    else
    {
        from_y_ -= half_size;
    }

    if (to_y_ >= (image_height - half_size))
    {
        to_y_ = image_height - (parameters.border_mode == cvpg::imageproc::algorithms::border_mode::ignore ? half_size : 0);
    }
    else
    {
        to_y_ += half_size;
    }

    if (size == 3)
    {
        if (parameters.border_mode == cvpg::imageproc::algorithms::border_mode::ignore)
        {
            scharr_gray_8bit_kernel_3x3_ignore_border(src, dst, from_x_, to_x_, from_y_, to_y_, image_width, mode);
        }
        else if (parameters.border_mode == cvpg::imageproc::algorithms::border_mode::constant)
        {
            scharr_gray_8bit_kernel_3x3_constant_border(src, dst, from_x_, to_x_, from_y_, to_y_, image_width, image_height, mode);
        }
        else if (parameters.border_mode == cvpg::imageproc::algorithms::border_mode::mirror)
        {
            scharr_gray_8bit_kernel_3x3_mirror_border(src, dst, from_x_, to_x_, from_y_, to_y_, image_width, image_height, mode);
        }
    }
    else
    {
        // TODO error handling
    }
}

}}} // namespace cvpg::imageproc::algorithms
