#include <libcvpg/imageproc/algorithms/tiling/sobel.hpp>

namespace {

void sobel_gray_8bit_kernel_3x3_ignore_border(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, std::size_t image_width, cvpg::imageproc::algorithms::sobel_operation_mode mode)
{
    std::uint8_t * src_line_m1 = nullptr;  // begin of previous line in source image
    std::uint8_t * src_line    = nullptr;  // begin of current line in source image
    std::uint8_t * src_line_p1 = nullptr;  // begin of next line in source image

    std::uint8_t * dst_line = nullptr;

    if (mode == cvpg::imageproc::algorithms::sobel_operation_mode::horizontal)
    {
        //
        // perform D = Gx * S
        //
        //      | g00 g01 g02 |   | +1  0 -1 |
        // Gx = | g10 g11 g12 | = | +2  0 -2 |
        //      | g20 g21 g22 |   | +1  0 -1 |
        //
        //      | s00 s01 s02 |
        // S  = | s10 s11 s12 | ; a11 = current image pixel
        //      | s20 s21 s22 |
        //

        for (std::size_t y = from_y; y < to_y; ++y)
        {
            const std::size_t offset_ym1 = image_width * (y - 1);   // offset previous line
            const std::size_t offset_y0  = image_width * y;         // offset current line
            const std::size_t offset_yp1 = image_width * (y + 1);   // offset next line

            src_line_m1 = src + offset_ym1;
            src_line    = src + offset_y0;
            src_line_p1 = src + offset_yp1;

            dst_line = dst + offset_y0;

            for (std::size_t x = from_x; x < to_x; ++x)
            {
                const std::int16_t d00 = src_line_m1[x - 1];
                const std::int16_t d02 = src_line_m1[x + 1];
                const std::int16_t d10 = src_line[x - 1] << 1; // 's10' * 2
                const std::int16_t d12 = src_line[x + 1] << 1; // 's12' * 2
                const std::int16_t d20 = src_line_p1[x - 1];
                const std::int16_t d22 = src_line_p1[x + 1];

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
    else if (mode == cvpg::imageproc::algorithms::sobel_operation_mode::vertical)
    {
        //
        // perform D = Gy * S
        //
        //      | g00 g01 g02 |   | +1 +2 +1 |
        // Gy = | g10 g11 g12 | = |  0  0  0 |
        //      | g20 g21 g22 |   | -1 -2 -1 |
        //
        //      | s00 s01 s02 |
        // S  = | s10 s11 s12 | ; a11 = current image pixel
        //      | s20 s21 s22 |
        //

        for (std::size_t y = from_y; y < to_y; ++y)
        {
            const std::size_t offset_ym1 = image_width * (y - 1);   // offset previous line
            const std::size_t offset_y0  = image_width * y;         // offset current line
            const std::size_t offset_yp1 = image_width * (y + 1);   // offset next line

            src_line_m1 = src + offset_ym1;
            src_line    = src + offset_y0;
            src_line_p1 = src + offset_yp1;

            dst_line = dst + offset_y0;

            for (std::size_t x = from_x; x < to_x; ++x)
            {
                const std::int16_t d00 = src_line_m1[x - 1];
                const std::int16_t d01 = src_line_m1[x] << 1; // 's01' * 2
                const std::int16_t d02 = src_line_m1[x + 1];
                const std::int16_t d20 = src_line_p1[x - 1];
                const std::int16_t d21 = src_line_p1[x] << 1; // 's01' * 2
                const std::int16_t d22 = src_line_p1[x + 1];

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

void sobel_gray_8bit_kernel_3x3_constant_border(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, std::size_t image_width, std::size_t image_height, cvpg::imageproc::algorithms::sobel_operation_mode mode)
{
    std::uint8_t * src_line_m1 = nullptr;  // begin of previous line in source image
    std::uint8_t * src_line    = nullptr;  // begin of current line in source image
    std::uint8_t * src_line_p1 = nullptr;  // begin of next line in source image

    std::uint8_t * dst_line = nullptr;

    if (mode == cvpg::imageproc::algorithms::sobel_operation_mode::horizontal)
    {
        //
        // perform D = Gx * S
        //
        //      | g00 g01 g02 |   | +1  0 -1 |
        // Gx = | g10 g11 g12 | = | +2  0 -2 |
        //      | g20 g21 g22 |   | +1  0 -1 |
        //
        //      | s00 s01 s02 |
        // S  = | s10 s11 s12 | ; a11 = current image pixel
        //      | s20 s21 s22 |
        //

        for (std::size_t y = from_y; y < to_y; ++y)
        {
            const std::size_t offset_ym1 = image_width * (y - 1);   // offset previous line
            const std::size_t offset_y0  = image_width * y;         // offset current line
            const std::size_t offset_yp1 = image_width * (y + 1);   // offset next line

            src_line_m1 = y != 0 ? (src + offset_ym1) : nullptr;
            src_line    = src + offset_y0;
            src_line_p1 = y != (image_height - 1) ? (src + offset_yp1) : nullptr;

            dst_line = dst + offset_y0;

            for (std::size_t x = from_x; x < to_x; ++x)
            {
                const std::int16_t d00 = src_line_m1 != nullptr ? (x != 0 ? src_line_m1[x - 1] : 0) : 0;
                const std::int16_t d02 = src_line_m1 != nullptr ? (x != (image_width - 1) ? src_line_m1[x + 1] : 0) : 0;
                const std::int16_t d10 = src_line[x - 1] << 1; // 's10' * 2
                const std::int16_t d12 = src_line[x + 1] << 1; // 's12' * 2
                const std::int16_t d20 = src_line_p1 != nullptr ? (x != 0 ? src_line_p1[x - 1] : 0) : 0;
                const std::int16_t d22 = src_line_p1 != nullptr ? (x != (image_width - 1) ? src_line_p1[x + 1] : 0) : 0;

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
    else if (mode == cvpg::imageproc::algorithms::sobel_operation_mode::vertical)
    {
        //
        // perform D = Gy * S
        //
        //      | g00 g01 g02 |   | +1 +2 +1 |
        // Gy = | g10 g11 g12 | = |  0  0  0 |
        //      | g20 g21 g22 |   | -1 -2 -1 |
        //
        //      | s00 s01 s02 |
        // S  = | s10 s11 s12 | ; a11 = current image pixel
        //      | s20 s21 s22 |
        //

        for (std::size_t y = from_y; y < to_y; ++y)
        {
            const std::size_t offset_ym1 = image_width * (y - 1);   // offset previous line
            const std::size_t offset_y0  = image_width * y;         // offset current line
            const std::size_t offset_yp1 = image_width * (y + 1);   // offset next line

            src_line_m1 = y != 0 ? (src + offset_ym1) : nullptr;
            src_line    = src + offset_y0;
            src_line_p1 = y != (image_height - 1) ? (src + offset_yp1) : nullptr;

            dst_line = dst + offset_y0;

            for (std::size_t x = from_x; x < to_x; ++x)
            {
                const std::int16_t d00 = src_line_m1 != nullptr ? (x != 0 ? src_line_m1[x - 1] : 0) : 0;
                const std::int16_t d01 = src_line_m1 != nullptr ? (src_line_m1[x] << 1) : 0; // 's01' * 2
                const std::int16_t d02 = src_line_m1 != nullptr ? (x != (image_width - 1) ? src_line_m1[x + 1] : 0) : 0;
                const std::int16_t d20 = src_line_p1 != nullptr ? (x != 0 ? src_line_p1[x - 1] : 0) : 0;
                const std::int16_t d21 = src_line_p1 != nullptr ? (src_line_p1[x] << 1) : 0; // 's01' * 2
                const std::int16_t d22 = src_line_p1 != nullptr ? (x != (image_width - 1) ? src_line_p1[x + 1] : 0) : 0;

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

void sobel_gray_8bit_kernel_3x3_mirror_border(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, std::size_t image_width, std::size_t image_height, cvpg::imageproc::algorithms::sobel_operation_mode mode)
{
    std::uint8_t * src_line_m1 = nullptr;  // begin of previous line in source image
    std::uint8_t * src_line    = nullptr;  // begin of current line in source image
    std::uint8_t * src_line_p1 = nullptr;  // begin of next line in source image

    std::uint8_t * dst_line = nullptr;

    if (mode == cvpg::imageproc::algorithms::sobel_operation_mode::horizontal)
    {
        //
        // perform D = Gx * S
        //
        //      | g00 g01 g02 |   | +1  0 -1 |
        // Gx = | g10 g11 g12 | = | +2  0 -2 |
        //      | g20 g21 g22 |   | +1  0 -1 |
        //
        //      | s00 s01 s02 |
        // S  = | s10 s11 s12 | ; a11 = current image pixel
        //      | s20 s21 s22 |
        //

        for (std::size_t y = from_y; y < to_y; ++y)
        {
            const std::size_t offset_ym1 = y != 0 ? (image_width * (y - 1)) : (image_width * (image_height - 1));     // offset previous line
            const std::size_t offset_y0  = image_width * y;                                                             // offset current line
            const std::size_t offset_yp1 = y != (image_height - 1) ? (image_width * (y + 1)) : 0;                     // offset next line

            src_line_m1 = src + offset_ym1;
            src_line    = src + offset_y0;
            src_line_p1 = src + offset_yp1;

            dst_line = dst + offset_y0;

            for (std::size_t x = from_x; x < to_x; ++x)
            {
                const std::int16_t d00 = src_line_m1[x != 0 ? (x - 1) : (image_width - 1)];
                const std::int16_t d02 = src_line_m1[x != (image_width - 1) ? (x + 1) : 0];
                const std::int16_t d10 = src_line[x != 0 ? (x - 1) : (image_width - 1)] << 1; // 's10' * 2
                const std::int16_t d12 = src_line[x != (image_width - 1) ? (x + 1) : 0] << 1; // 's12' * 2
                const std::int16_t d20 = src_line_p1[x != 0 ? (x - 1) : (image_width - 1)];
                const std::int16_t d22 = src_line_p1[x != (image_width - 1) ? (x + 1) : 0];

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
    else if (mode == cvpg::imageproc::algorithms::sobel_operation_mode::vertical)
    {
        //
        // perform D = Gy * S
        //
        //      | g00 g01 g02 |   | +1 +2 +1 |
        // Gy = | g10 g11 g12 | = |  0  0  0 |
        //      | g20 g21 g22 |   | -1 -2 -1 |
        //
        //      | s00 s01 s02 |
        // S  = | s10 s11 s12 | ; a11 = current image pixel
        //      | s20 s21 s22 |
        //

        for (std::size_t y = from_y; y < to_y; ++y)
        {
            const std::size_t offset_ym1 = y != 0 ? (image_width * (y - 1)) : (image_width * (image_height - 1));     // offset previous line
            const std::size_t offset_y0  = image_width * y;                                                             // offset current line
            const std::size_t offset_yp1 = y != (image_height - 1) ? (image_width * (y + 1)) : 0;                     // offset next line

            src_line_m1 = src + offset_ym1;
            src_line    = src + offset_y0;
            src_line_p1 = src + offset_yp1;

            dst_line = dst + offset_y0;

            for (std::size_t x = from_x; x < to_x; ++x)
            {
                const std::int16_t d00 = src_line_m1[x != 0 ? (x - 1) : (image_width - 1)];
                const std::int16_t d01 = src_line_m1[x] << 1; // 's01' * 2
                const std::int16_t d02 = src_line_m1[x != (image_width - 1) ? (x + 1) : 0];
                const std::int16_t d20 = src_line_p1[x != 0 ? (x - 1) : (image_width - 1)];
                const std::int16_t d21 = src_line_p1[x] << 1; // 's01' * 2
                const std::int16_t d22 = src_line_p1[x != (image_width - 1) ? (x + 1) : 0];

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

void sobel_gray_8bit_kernel_5x5_ignore_border(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, std::size_t image_width, cvpg::imageproc::algorithms::sobel_operation_mode mode)
{
    std::uint8_t * src_line_m2 = nullptr;  // begin of previous previous line in source image
    std::uint8_t * src_line_m1 = nullptr;  // begin of previous line in source image
    std::uint8_t * src_line    = nullptr;  // begin of current line in source image
    std::uint8_t * src_line_p1 = nullptr;  // begin of next line in source image
    std::uint8_t * src_line_p2 = nullptr;  // begin of next next line in source image

    std::uint8_t * dst_line = nullptr;

    if (mode == cvpg::imageproc::algorithms::sobel_operation_mode::horizontal)
    {
        //
        // perform D = Gx * S
        //
        //      | g00 g01 g02 g03 g04 |   | +2 +1  0 -1 -2 |
        //      | g10 g11 g02 g13 g14 |   | +2 +1  0 -1 -2 |
        // Gx = | g20 g21 g22 g23 g24 | = | +4 +2  0 -2 -4 |
        //      | g30 g31 g32 g33 g34 |   | +2 +1  0 -1 -2 |
        //      | g40 g41 g42 g43 g44 |   | +2 +1  0 -1 -2 |
        //
        //      | s00 s01 s02 s03 s04 |
        //      | s10 s11 s12 s13 s14 |
        // S  = | s20 s21 s22 s23 s24 | ; a11 = current image pixel
        //      | s30 s31 s32 s33 s34 |
        //      | s40 s41 s42 s43 s44 |
        //

        for (std::size_t y = from_y; y < to_y; ++y)
        {
            const std::size_t offset_ym2 = image_width * (y - 2);   // offset previous previous line
            const std::size_t offset_ym1 = image_width * (y - 1);   // offset previous line
            const std::size_t offset_y0  = image_width * y;         // offset current line
            const std::size_t offset_yp1 = image_width * (y + 1);   // offset next line
            const std::size_t offset_yp2 = image_width * (y + 2);   // offset next next line

            src_line_m2 = src + offset_ym2;
            src_line_m1 = src + offset_ym1;
            src_line    = src + offset_y0;
            src_line_p1 = src + offset_yp1;
            src_line_p2 = src + offset_yp2;

            dst_line = dst + offset_y0;

            for (std::size_t x = from_x; x < to_x; ++x)
            {
                const std::int16_t d00 = src_line_m2[x - 2] << 1; // 's00' * 2
                const std::int16_t d01 = src_line_m2[x - 1];
                const std::int16_t d03 = src_line_m2[x + 1];
                const std::int16_t d04 = src_line_m2[x + 2] << 1; // 's04' * 2

                const std::int16_t d10 = src_line_m1[x - 2] << 1; // 's10' * 2
                const std::int16_t d11 = src_line_m1[x - 1];
                const std::int16_t d13 = src_line_m1[x + 1];
                const std::int16_t d14 = src_line_m1[x + 2] << 1; // 's14' * 2

                const std::int16_t d20 = src_line[x - 2] << 2; // 's20' * 4
                const std::int16_t d21 = src_line[x - 1] << 1; // 's21' * 2
                const std::int16_t d23 = src_line[x + 1] << 1; // 's23' * 2
                const std::int16_t d24 = src_line[x + 2] << 2; // 's24' * 4

                const std::int16_t d30 = src_line_p1[x - 2] << 1; // 's30' * 2
                const std::int16_t d31 = src_line_p1[x - 1];
                const std::int16_t d33 = src_line_p1[x + 1];
                const std::int16_t d34 = src_line_p1[x + 2] << 1; // 's34' * 2

                const std::int16_t d40 = src_line_p2[x - 2] << 1; // 's40' * 2
                const std::int16_t d41 = src_line_p2[x - 1];
                const std::int16_t d43 = src_line_p2[x + 1];
                const std::int16_t d44 = src_line_p2[x + 2] << 1; // 's44' * 2

                const std::int16_t res = d00 + d01 - d03 - d04 +
                                         d10 + d11 - d13 - d14 +
                                         d20 + d21 - d23 - d24 +
                                         d30 + d31 - d33 - d34 +
                                         d40 + d41 - d43 - d44;

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
    else if (mode == cvpg::imageproc::algorithms::sobel_operation_mode::vertical)
    {
        //
        // perform D = Gy * S
        //
        //      | g00 g01 g02 g03 g04 |   | +2 +2 +4 +2 +2 |
        //      | g10 g11 g02 g13 g14 |   | +1 +1 +2 +1 +1 |
        // Gy = | g20 g21 g22 g23 g24 | = |  0  0  0  0  0 |
        //      | g30 g31 g32 g33 g34 |   | -2 -1  0 -1 -2 |
        //      | g40 g41 g42 g43 g44 |   | +2 +1  0 -1 -2 |
        //
        //      | s00 s01 s02 s03 s04 |
        //      | s10 s11 s12 s13 s14 |
        // S  = | s20 s21 s22 s23 s24 | ; a11 = current image pixel
        //      | s30 s31 s32 s33 s34 |
        //      | s40 s41 s42 s43 s44 |
        //

        for (std::size_t y = from_y; y < to_y; ++y)
        {
            const std::size_t offset_ym2 = image_width * (y - 2);   // offset previous previous line
            const std::size_t offset_ym1 = image_width * (y - 1);   // offset previous line
            const std::size_t offset_y0  = image_width * y;         // offset current line
            const std::size_t offset_yp1 = image_width * (y + 1);   // offset next line
            const std::size_t offset_yp2 = image_width * (y + 2);   // offset next next line

            src_line_m2 = src + offset_ym2;
            src_line_m1 = src + offset_ym1;
            src_line    = src + offset_y0;
            src_line_p1 = src + offset_yp1;
            src_line_p2 = src + offset_yp2;

            dst_line = dst + offset_y0;

            for (std::size_t x = from_x; x < to_x; ++x)
            {
                const std::int16_t d00 = src_line_m2[x - 2] << 1; // 's00' * 2
                const std::int16_t d01 = src_line_m2[x - 1] << 1; // 's01' * 2
                const std::int16_t d02 = src_line_m2[x] << 2; // 's02' * 4
                const std::int16_t d03 = src_line_m2[x + 1] << 1; // 's03' * 2
                const std::int16_t d04 = src_line_m2[x + 2] << 1; // 's04' * 2

                const std::int16_t d10 = src_line_m1[x - 2];
                const std::int16_t d11 = src_line_m1[x - 1];
                const std::int16_t d12 = src_line_m1[x] << 1; // 's12' * 2
                const std::int16_t d13 = src_line_m1[x + 1];
                const std::int16_t d14 = src_line_m1[x + 2];

                const std::int16_t d30 = src_line_p1[x - 2];
                const std::int16_t d31 = src_line_p1[x - 1];
                const std::int16_t d32 = src_line_p1[x] << 1; // 's32' * 2
                const std::int16_t d33 = src_line_p1[x + 1];
                const std::int16_t d34 = src_line_p1[x + 2];

                const std::int16_t d40 = src_line_p2[x - 2] << 1; // 's40' * 2
                const std::int16_t d41 = src_line_p2[x - 1] << 1; // 's41' * 2
                const std::int16_t d42 = src_line_p2[x] << 2; // 's42' * 4
                const std::int16_t d43 = src_line_p2[x + 1] << 1; // 's43' * 2
                const std::int16_t d44 = src_line_p2[x + 2] << 1; // 's44' * 2

                const std::int16_t res = d00 + d01 + d02 + d03 + d04 +
                                         d10 + d11 + d12 + d13 + d14 -
                                         d30 - d31 - d32 - d33 - d34 -
                                         d40 - d41 - d42 - d43 - d44;

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

void sobel_gray_8bit_kernel_5x5_constant_border(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, std::size_t image_width, std::size_t image_height, cvpg::imageproc::algorithms::sobel_operation_mode mode)
{
    std::uint8_t * src_line_m2 = nullptr;  // begin of previous previous line in source image
    std::uint8_t * src_line_m1 = nullptr;  // begin of previous line in source image
    std::uint8_t * src_line    = nullptr;  // begin of current line in source image
    std::uint8_t * src_line_p1 = nullptr;  // begin of next line in source image
    std::uint8_t * src_line_p2 = nullptr;  // begin of next next line in source image

    std::uint8_t * dst_line = nullptr;

    if (mode == cvpg::imageproc::algorithms::sobel_operation_mode::horizontal)
    {
        //
        // perform D = Gx * S
        //
        //      | g00 g01 g02 g03 g04 |   | +2 +1  0 -1 -2 |
        //      | g10 g11 g02 g13 g14 |   | +2 +1  0 -1 -2 |
        // Gx = | g20 g21 g22 g23 g24 | = | +4 +2  0 -2 -4 |
        //      | g30 g31 g32 g33 g34 |   | +2 +1  0 -1 -2 |
        //      | g40 g41 g42 g43 g44 |   | +2 +1  0 -1 -2 |
        //
        //      | s00 s01 s02 s03 s04 |
        //      | s10 s11 s12 s13 s14 |
        // S  = | s20 s21 s22 s23 s24 | ; a11 = current image pixel
        //      | s30 s31 s32 s33 s34 |
        //      | s40 s41 s42 s43 s44 |
        //

        for (std::size_t y = from_y; y < to_y; ++y)
        {
            const std::size_t offset_ym2 = image_width * (y - 2);   // offset previous previous line
            const std::size_t offset_ym1 = image_width * (y - 1);   // offset previous line
            const std::size_t offset_y0  = image_width * y;         // offset current line
            const std::size_t offset_yp1 = image_width * (y + 1);   // offset next line
            const std::size_t offset_yp2 = image_width * (y + 2);   // offset next next line

            src_line_m2 = y > 1 ? (src + offset_ym2) : nullptr;
            src_line_m1 = y != 0 ? (src + offset_ym1) : nullptr;
            src_line    = src + offset_y0;
            src_line_p1 = y != (image_height - 1) ? (src + offset_yp1) : nullptr;
            src_line_p2 = y < (image_height - 2) ? (src + offset_yp2) : nullptr;

            dst_line = dst + offset_y0;

            for (std::size_t x = from_x; x < to_x; ++x)
            {
                const std::int16_t d00 = src_line_m2 != nullptr ? (src_line_m2[x - 2] << 1) : 0; // 's00' * 2
                const std::int16_t d01 = src_line_m2 != nullptr ? (src_line_m2[x - 1]) : 0;
                const std::int16_t d03 = src_line_m2 != nullptr ? (src_line_m2[x + 1]) : 0;
                const std::int16_t d04 = src_line_m2 != nullptr ? (src_line_m2[x + 2] << 1) : 0; // 's04' * 2

                const std::int16_t d10 = src_line_m1 != nullptr ? (src_line_m1[x - 2] << 1) : 0; // 's10' * 2
                const std::int16_t d11 = src_line_m1 != nullptr ? (src_line_m1[x - 1]) : 0;
                const std::int16_t d13 = src_line_m1 != nullptr ? (src_line_m1[x + 1]) : 0;
                const std::int16_t d14 = src_line_m1 != nullptr ? (src_line_m1[x + 2] << 1) : 0; // 's14' * 2

                const std::int16_t d20 = src_line[x - 2] << 2; // 's20' * 4
                const std::int16_t d21 = src_line[x - 1] << 1; // 's21' * 2
                const std::int16_t d23 = src_line[x + 1] << 1; // 's23' * 2
                const std::int16_t d24 = src_line[x + 2] << 2; // 's24' * 4

                const std::int16_t d30 = src_line_p1 != nullptr ? (src_line_p1[x - 2] << 1) : 0; // 's30' * 2
                const std::int16_t d31 = src_line_p1 != nullptr ? (src_line_p1[x - 1]) : 0;
                const std::int16_t d33 = src_line_p1 != nullptr ? (src_line_p1[x + 1]) : 0;
                const std::int16_t d34 = src_line_p1 != nullptr ? (src_line_p1[x + 2] << 1) : 0; // 's34' * 2

                const std::int16_t d40 = src_line_p2 != nullptr ? (src_line_p2[x - 2] << 1) : 0; // 's40' * 2
                const std::int16_t d41 = src_line_p2 != nullptr ? (src_line_p2[x - 1]) : 0;
                const std::int16_t d43 = src_line_p2 != nullptr ? (src_line_p2[x + 1]) : 0;
                const std::int16_t d44 = src_line_p2 != nullptr ? (src_line_p2[x + 2] << 1) : 0; // 's44' * 2

                const std::int16_t res = d00 + d01 - d03 - d04 +
                                         d10 + d11 - d13 - d14 +
                                         d20 + d21 - d23 - d24 +
                                         d30 + d31 - d33 - d34 +
                                         d40 + d41 - d43 - d44;

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
    else if (mode == cvpg::imageproc::algorithms::sobel_operation_mode::vertical)
    {
        //
        // perform D = Gy * S
        //
        //      | g00 g01 g02 g03 g04 |   | +2 +2 +4 +2 +2 |
        //      | g10 g11 g02 g13 g14 |   | +1 +1 +2 +1 +1 |
        // Gy = | g20 g21 g22 g23 g24 | = |  0  0  0  0  0 |
        //      | g30 g31 g32 g33 g34 |   | -2 -1  0 -1 -2 |
        //      | g40 g41 g42 g43 g44 |   | +2 +1  0 -1 -2 |
        //
        //      | s00 s01 s02 s03 s04 |
        //      | s10 s11 s12 s13 s14 |
        // S  = | s20 s21 s22 s23 s24 | ; a11 = current image pixel
        //      | s30 s31 s32 s33 s34 |
        //      | s40 s41 s42 s43 s44 |
        //

        for (std::size_t y = from_y; y < to_y; ++y)
        {
            const std::size_t offset_ym2 = image_width * (y - 2);   // offset previous previous line
            const std::size_t offset_ym1 = image_width * (y - 1);   // offset previous line
            const std::size_t offset_y0  = image_width * y;         // offset current line
            const std::size_t offset_yp1 = image_width * (y + 1);   // offset next line
            const std::size_t offset_yp2 = image_width * (y + 2);   // offset next next line

            src_line_m2 = y > 1 ? (src + offset_ym2) : nullptr;
            src_line_m1 = y != 0 ? (src + offset_ym1) : nullptr;
            src_line    = src + offset_y0;
            src_line_p1 = y != (image_height - 1) ? (src + offset_yp1) : nullptr;
            src_line_p2 = y < (image_height - 2) ? (src + offset_yp2) : nullptr;

            dst_line = dst + offset_y0;

            for (std::size_t x = from_x; x < to_x; ++x)
            {
                const std::int16_t d00 = src_line_m2 != nullptr ? (src_line_m2[x - 2] << 1) : 0; // 's00' * 2
                const std::int16_t d01 = src_line_m2 != nullptr ? (src_line_m2[x - 1] << 1) : 0; // 's01' * 2
                const std::int16_t d02 = src_line_m2 != nullptr ? (src_line_m2[x] << 2) : 0; // 's02' * 4
                const std::int16_t d03 = src_line_m2 != nullptr ? (src_line_m2[x + 1] << 1) : 0; // 's03' * 2
                const std::int16_t d04 = src_line_m2 != nullptr ? (src_line_m2[x + 2] << 1) : 0; // 's04' * 2

                const std::int16_t d10 = src_line_m1 != nullptr ? src_line_m1[x - 2] : 0;
                const std::int16_t d11 = src_line_m1 != nullptr ? src_line_m1[x - 1] : 0;
                const std::int16_t d12 = src_line_m1 != nullptr ? (src_line_m1[x] << 1) : 0; // 's12' * 2
                const std::int16_t d13 = src_line_m1 != nullptr ? src_line_m1[x + 1] : 0;
                const std::int16_t d14 = src_line_m1 != nullptr ? src_line_m1[x + 2] : 0;

                const std::int16_t d30 = src_line_p1 != nullptr ? src_line_p1[x - 2] : 0;
                const std::int16_t d31 = src_line_p1 != nullptr ? src_line_p1[x - 1] : 0;
                const std::int16_t d32 = src_line_p1 != nullptr ? (src_line_p1[x] << 1) : 0; // 's32' * 2
                const std::int16_t d33 = src_line_p1 != nullptr ? src_line_p1[x + 1] : 0;
                const std::int16_t d34 = src_line_p1 != nullptr ? src_line_p1[x + 2] : 0;

                const std::int16_t d40 = src_line_p2 != nullptr ? (src_line_p2[x - 2] << 1) : 0; // 's40' * 2
                const std::int16_t d41 = src_line_p2 != nullptr ? (src_line_p2[x - 1] << 1) : 0; // 's41' * 2
                const std::int16_t d42 = src_line_p2 != nullptr ? (src_line_p2[x] << 2) : 0; // 's42' * 4
                const std::int16_t d43 = src_line_p2 != nullptr ? (src_line_p2[x + 1] << 1) : 0; // 's43' * 2
                const std::int16_t d44 = src_line_p2 != nullptr ? (src_line_p2[x + 2] << 1) : 0; // 's44' * 2

                const std::int16_t res = d00 + d01 + d02 + d03 + d04 +
                                         d10 + d11 + d12 + d13 + d14 -
                                         d30 - d31 - d32 - d33 - d34 -
                                         d40 - d41 - d42 - d43 - d44;

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

void sobel_gray_8bit_kernel_5x5_mirror_border(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, std::size_t image_width, std::size_t image_height, cvpg::imageproc::algorithms::sobel_operation_mode mode)
{
    std::uint8_t * src_line_m2 = nullptr;  // begin of previous previous line in source image
    std::uint8_t * src_line_m1 = nullptr;  // begin of previous line in source image
    std::uint8_t * src_line    = nullptr;  // begin of current line in source image
    std::uint8_t * src_line_p1 = nullptr;  // begin of next line in source image
    std::uint8_t * src_line_p2 = nullptr;  // begin of next next line in source image

    std::uint8_t * dst_line = nullptr;

    if (mode == cvpg::imageproc::algorithms::sobel_operation_mode::horizontal)
    {
        //
        // perform D = Gx * S
        //
        //      | g00 g01 g02 g03 g04 |   | +2 +1  0 -1 -2 |
        //      | g10 g11 g02 g13 g14 |   | +2 +1  0 -1 -2 |
        // Gx = | g20 g21 g22 g23 g24 | = | +4 +2  0 -2 -4 |
        //      | g30 g31 g32 g33 g34 |   | +2 +1  0 -1 -2 |
        //      | g40 g41 g42 g43 g44 |   | +2 +1  0 -1 -2 |
        //
        //      | s00 s01 s02 s03 s04 |
        //      | s10 s11 s12 s13 s14 |
        // S  = | s20 s21 s22 s23 s24 | ; a11 = current image pixel
        //      | s30 s31 s32 s33 s34 |
        //      | s40 s41 s42 s43 s44 |
        //

        for (std::size_t y = from_y; y < to_y; ++y)
        {
            const std::size_t offset_ym2 = y > 1 ? (image_width * (y - 2)) : (image_width * (image_height - (2 - y)));  // offset previous previous line
            const std::size_t offset_ym1 = y != 0 ? (image_width * (y - 1)) : (image_width * (image_height - 1));       // offset previous line
            const std::size_t offset_y0  = image_width * y;                                                             // offset current line
            const std::size_t offset_yp1 = y != (image_height - 1) ? (image_width * (y + 1)) : 0;                       // offset next line
            const std::size_t offset_yp2 = y < (image_height - 2) ? (image_width * (y + 2)) : (2 - y);                  // offset next next line

            src_line_m2 = src + offset_ym2;
            src_line_m1 = src + offset_ym1;
            src_line    = src + offset_y0;
            src_line_p1 = src + offset_yp1;
            src_line_p2 = src + offset_yp2;

            dst_line = dst + offset_y0;

            for (std::size_t x = from_x; x < to_x; ++x)
            {
                const std::int16_t d00 = src_line_m2[x > 1 ? (x - 2) : (image_width - (2 - y))] << 1; // 's00' * 2
                const std::int16_t d01 = src_line_m2[x != 0 ? (x - 1) : (image_width - 1)];
                const std::int16_t d03 = src_line_m2[x != (image_width - 1) ? (x + 1) : 0];
                const std::int16_t d04 = src_line_m2[x < (image_width - 2) ? (x + 2) : (2 - y)] << 1; // 's04' * 2

                const std::int16_t d10 = src_line_m1[x > 1 ? (x - 2) : (image_width - (2 - y))] << 1; // 's10' * 2
                const std::int16_t d11 = src_line_m1[x != 0 ? (x - 1) : (image_width - 1)];
                const std::int16_t d13 = src_line_m1[x != (image_width - 1) ? (x + 1) : 0];
                const std::int16_t d14 = src_line_m1[x < (image_width - 2) ? (x + 2) : (2 - y)] << 1; // 's14' * 2

                const std::int16_t d20 = src_line[x > 1 ? (x - 2) : (image_width - (2 - y))] << 2; // 's20' * 4
                const std::int16_t d21 = src_line[x != 0 ? (x - 1) : (image_width - 1)] << 1; // 's21' * 2
                const std::int16_t d23 = src_line[x != (image_width - 1) ? (x + 1) : 0] << 1; // 's23' * 2
                const std::int16_t d24 = src_line[x < (image_width - 2) ? (x + 2) : (2 - y)] << 2; // 's24' * 4

                const std::int16_t d30 = src_line_p1[x > 1 ? (x - 2) : (image_width - (2 - y))] << 1; // 's30' * 2
                const std::int16_t d31 = src_line_p1[x != 0 ? (x - 1) : (image_width - 1)];
                const std::int16_t d33 = src_line_p1[x != (image_width - 1) ? (x + 1) : 0];
                const std::int16_t d34 = src_line_p1[x < (image_width - 2) ? (x + 2) : (2 - y)] << 1; // 's34' * 2

                const std::int16_t d40 = src_line_p2[x > 1 ? (x - 2) : (image_width - (2 - y))] << 1; // 's40' * 2
                const std::int16_t d41 = src_line_p2[x != 0 ? (x - 1) : (image_width - 1)];
                const std::int16_t d43 = src_line_p2[x != (image_width - 1) ? (x + 1) : 0];
                const std::int16_t d44 = src_line_p2[x < (image_width - 2) ? (x + 2) : (2 - y)] << 1; // 's44' * 2

                const std::int16_t res = d00 + d01 - d03 - d04 +
                                         d10 + d11 - d13 - d14 +
                                         d20 + d21 - d23 - d24 +
                                         d30 + d31 - d33 - d34 +
                                         d40 + d41 - d43 - d44;

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
    else if (mode == cvpg::imageproc::algorithms::sobel_operation_mode::vertical)
    {
        //
        // perform D = Gy * S
        //
        //      | g00 g01 g02 g03 g04 |   | +2 +2 +4 +2 +2 |
        //      | g10 g11 g02 g13 g14 |   | +1 +1 +2 +1 +1 |
        // Gy = | g20 g21 g22 g23 g24 | = |  0  0  0  0  0 |
        //      | g30 g31 g32 g33 g34 |   | -2 -1  0 -1 -2 |
        //      | g40 g41 g42 g43 g44 |   | +2 +1  0 -1 -2 |
        //
        //      | s00 s01 s02 s03 s04 |
        //      | s10 s11 s12 s13 s14 |
        // S  = | s20 s21 s22 s23 s24 | ; a11 = current image pixel
        //      | s30 s31 s32 s33 s34 |
        //      | s40 s41 s42 s43 s44 |
        //

        for (std::size_t y = from_y; y < to_y; ++y)
        {
            const std::size_t offset_ym2 = y > 1 ? (image_width * (y - 2)) : (image_width * (image_height - (2 - y)));  // offset previous previous line
            const std::size_t offset_ym1 = y != 0 ? (image_width * (y - 1)) : (image_width * (image_height - 1));       // offset previous line
            const std::size_t offset_y0  = image_width * y;                                                             // offset current line
            const std::size_t offset_yp1 = y != (image_height - 1) ? (image_width * (y + 1)) : 0;                       // offset next line
            const std::size_t offset_yp2 = y < (image_height - 2) ? (image_width * (y + 2)) : (2 - y);                  // offset next next line

            src_line_m2 = src + offset_ym2;
            src_line_m1 = src + offset_ym1;
            src_line    = src + offset_y0;
            src_line_p1 = src + offset_yp1;
            src_line_p2 = src + offset_yp2;

            dst_line = dst + offset_y0;

            for (std::size_t x = from_x; x < to_x; ++x)
            {
                const std::int16_t d00 = src_line_m2[x > 1 ? (x - 2) : (image_width - (2 - y))] << 1; // 's00' * 2
                const std::int16_t d01 = src_line_m2[x != 0 ? (x - 1) : (image_width - 1)] << 1; // 's01' * 2
                const std::int16_t d02 = src_line_m2[x] << 2; // 's02' * 4
                const std::int16_t d03 = src_line_m2[x + 1] << 1; // 's03' * 2
                const std::int16_t d04 = src_line_m2[x < (image_width - 2) ? (x + 2) : (2 - y)] << 1; // 's04' * 2

                const std::int16_t d10 = src_line_m1[x > 1 ? (x - 2) : (image_width - (2 - y))];
                const std::int16_t d11 = src_line_m1[x != 0 ? (x - 1) : (image_width - 1)];
                const std::int16_t d12 = src_line_m1[x] << 1; // 's12' * 2
                const std::int16_t d13 = src_line_m1[x != (image_width - 1) ? (x + 1) : 0];
                const std::int16_t d14 = src_line_m1[x < (image_width - 2) ? (x + 2) : (2 - y)];

                const std::int16_t d30 = src_line_p1[x > 1 ? (x - 2) : (image_width - (2 - y))];
                const std::int16_t d31 = src_line_p1[x != 0 ? (x - 1) : (image_width - 1)];
                const std::int16_t d32 = src_line_p1[x] << 1; // 's32' * 2
                const std::int16_t d33 = src_line_p1[x != (image_width - 1) ? (x + 1) : 0];
                const std::int16_t d34 = src_line_p1[x < (image_width - 2) ? (x + 2) : (2 - y)];

                const std::int16_t d40 = src_line_p2[x > 1 ? (x - 2) : (image_width - (2 - y))] << 1; // 's40' * 2
                const std::int16_t d41 = src_line_p2[x != 0 ? (x - 1) : (image_width - 1)] << 1; // 's41' * 2
                const std::int16_t d42 = src_line_p2[x] << 2; // 's42' * 4
                const std::int16_t d43 = src_line_p2[x != (image_width - 1) ? (x + 1) : 0] << 1; // 's43' * 2
                const std::int16_t d44 = src_line_p2[x < (image_width - 2) ? (x + 2) : (2 - y)] << 1; // 's44' * 2

                const std::int16_t res = d00 + d01 + d02 + d03 + d04 +
                                         d10 + d11 + d12 + d13 + d14 -
                                         d30 - d31 - d32 - d33 - d34 -
                                         d40 - d41 - d42 - d43 - d44;

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

void sobel_gray_8bit(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters, sobel_operation_mode mode)
{
    const std::size_t image_width = parameters.image_width;
    const std::size_t image_height = parameters.image_height;

    const std::int32_t size = parameters.signed_integer_numbers.at(0);

    // correct from/to values ; they have to overlap to make sobel work correctly
    const std::size_t half_size = size >> 1;

    if (from_x <= half_size)
    {
        from_x = parameters.border_mode == cvpg::imageproc::algorithms::border_mode::ignore ? half_size : 0;
    }
    else
    {
        from_x -= half_size;
    }

    if (to_x >= (image_width - half_size))
    {
        to_x = image_width - (parameters.border_mode == cvpg::imageproc::algorithms::border_mode::ignore ? half_size : 0);
    }
    else
    {
        to_x += half_size;
    }

    if (from_y <= half_size)
    {
        from_y = parameters.border_mode == cvpg::imageproc::algorithms::border_mode::ignore ? half_size : 0;
    }
    else
    {
        from_y -= half_size;
    }

    if (to_y >= (image_height - half_size))
    {
        to_y = image_height - (parameters.border_mode == cvpg::imageproc::algorithms::border_mode::ignore ? half_size : 0);
    }
    else
    {
        to_y += half_size;
    }

    if (size == 3)
    {
        if (parameters.border_mode == cvpg::imageproc::algorithms::border_mode::ignore)
        {
            sobel_gray_8bit_kernel_3x3_ignore_border(src, dst, from_x, to_x, from_y, to_y, image_width, mode);
        }
        else if (parameters.border_mode == cvpg::imageproc::algorithms::border_mode::constant)
        {
            sobel_gray_8bit_kernel_3x3_constant_border(src, dst, from_x, to_x, from_y, to_y, image_width, image_height, mode);
        }
        else if (parameters.border_mode == cvpg::imageproc::algorithms::border_mode::mirror)
        {
            sobel_gray_8bit_kernel_3x3_mirror_border(src, dst, from_x, to_x, from_y, to_y, image_width, image_height, mode);
        }
    }
    else if (size == 5)
    {
        if (parameters.border_mode == cvpg::imageproc::algorithms::border_mode::ignore)
        {
            sobel_gray_8bit_kernel_5x5_ignore_border(src, dst, from_x, to_x, from_y, to_y, image_width, mode);
        }
        else if (parameters.border_mode == cvpg::imageproc::algorithms::border_mode::constant)
        {
            sobel_gray_8bit_kernel_5x5_constant_border(src, dst, from_x, to_x, from_y, to_y, image_width, image_height, mode);
        }
        else if (parameters.border_mode == cvpg::imageproc::algorithms::border_mode::mirror)
        {
            sobel_gray_8bit_kernel_5x5_mirror_border(src, dst, from_x, to_x, from_y, to_y, image_width, image_height, mode);
        }
    }
    else
    {
        // TODO error handling
    }
}

}}} // namespace cvpg::imageproc::algorithms
