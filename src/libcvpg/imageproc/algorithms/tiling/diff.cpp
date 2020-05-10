#include <libcvpg/imageproc/algorithms/tiling/diff.hpp>

#if defined __AVX2__

#include <immintrin.h>

#endif

#include <algorithm>

namespace {

#if defined __AVX2__

void diff_gray_8bit_avx2(std::uint8_t * src1, std::uint8_t * src2, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
{
    const std::size_t image_width = parameters.image_width;

    const std::int32_t offset = parameters.signed_integer_numbers.at(0);

    // set offset added after difference
    __m256 offset_ = _mm256_set1_ps(static_cast<float>(offset));

    // set mask to use for maximum check (values > 255)
    __m256i max_mask = _mm256_set1_epi32(255);

    // set mask to use for minimum check (values < 0)
    __m256i min_mask = _mm256_set1_epi32(0);

    // conversion union
    union res
    {
        __m256i       data;
        std::uint32_t ints[8];
    };

    res r, r1, r2, r3, r4;

    std::uint8_t * src1_line = nullptr;
    std::uint8_t * src2_line = nullptr;
    std::uint8_t * dst_line  = nullptr;

    for (std::size_t y = from_y; y <= to_y; ++y)
    {
        const std::size_t offset_y = image_width * y;

        src1_line = src1 + offset_y;
        src2_line = src2 + offset_y;
        dst_line  = dst  + offset_y;

        for (std::size_t x = from_x; x <= to_x; x += 32)
        {
            // load data from (unaligned) memory into 256-bit register
            __m256i data1 = _mm256_loadu_si256((__m256i const *)(src1_line + x));
            __m256i data2 = _mm256_loadu_si256((__m256i const *)(src2_line + x));

            // extract two 128-bit from 256-bit register
            __m128i data1_high = _mm256_extracti128_si256(data1, 1);
            __m128i data1_low  = _mm256_extracti128_si256(data1, 0);
            __m128i data2_high = _mm256_extracti128_si256(data2, 1);
            __m128i data2_low  = _mm256_extracti128_si256(data2, 0);

            // convert lower half of 128-bit registers from 8-bit values to 32-bit values
            __m256i converted1_high = _mm256_cvtepu8_epi32(data1_high);
            __m256i converted1_low  = _mm256_cvtepu8_epi32(data1_low);
            __m256i converted2_high = _mm256_cvtepu8_epi32(data2_high);
            __m256i converted2_low  = _mm256_cvtepu8_epi32(data2_low);

            // shuffle data to perform next extract & conversion
            __m256i data1_shuffled = _mm256_shuffle_epi8(data1, _mm256_setr_epi8( 8,  9, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,
                                                                                  8,  9, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0));
            __m256i data2_shuffled = _mm256_shuffle_epi8(data2, _mm256_setr_epi8( 8,  9, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,
                                                                                  8,  9, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0));

            // extract two 128-bit from 256-bit register
            __m128i data1_high_ = _mm256_extracti128_si256(data1_shuffled, 1);
            __m128i data1_low_  = _mm256_extracti128_si256(data1_shuffled, 0);
            __m128i data2_high_ = _mm256_extracti128_si256(data2_shuffled, 1);
            __m128i data2_low_  = _mm256_extracti128_si256(data2_shuffled, 0);

            // convert lower half of 128-bit registers from 8-bit values to 32-bit values
            __m256i converted1_high_ = _mm256_cvtepu8_epi32(data1_high_);
            __m256i converted1_low_  = _mm256_cvtepu8_epi32(data1_low_);
            __m256i converted2_high_ = _mm256_cvtepu8_epi32(data2_high_);
            __m256i converted2_low_  = _mm256_cvtepu8_epi32(data2_low_);

            // convert 32-bit integers into single precision floats
            __m256 floats11 = _mm256_cvtepi32_ps(converted1_high);
            __m256 floats12 = _mm256_cvtepi32_ps(converted1_low);
            __m256 floats13 = _mm256_cvtepi32_ps(converted1_high_);
            __m256 floats14 = _mm256_cvtepi32_ps(converted1_low_);
            __m256 floats21 = _mm256_cvtepi32_ps(converted2_high);
            __m256 floats22 = _mm256_cvtepi32_ps(converted2_low);
            __m256 floats23 = _mm256_cvtepi32_ps(converted2_high_);
            __m256 floats24 = _mm256_cvtepi32_ps(converted2_low_);

            // subtract floats
            __m256 sub1 = _mm256_sub_ps(floats11, floats21);
            __m256 sub2 = _mm256_sub_ps(floats12, floats22);
            __m256 sub3 = _mm256_sub_ps(floats13, floats23);
            __m256 sub4 = _mm256_sub_ps(floats14, floats24);

            // add offsets
            __m256 add1 = _mm256_add_ps(sub1, offset_);
            __m256 add2 = _mm256_add_ps(sub2, offset_);
            __m256 add3 = _mm256_add_ps(sub3, offset_);
            __m256 add4 = _mm256_add_ps(sub4, offset_);

            // convert floats back to 32-bit integers
            __m256i int1 = _mm256_cvtps_epi32(add1);
            __m256i int2 = _mm256_cvtps_epi32(add2);
            __m256i int3 = _mm256_cvtps_epi32(add3);
            __m256i int4 = _mm256_cvtps_epi32(add4);

            // saturate at 255
            __m256i int_max1 = _mm256_min_epi32(int1, max_mask);
            __m256i int_max2 = _mm256_min_epi32(int2, max_mask);
            __m256i int_max3 = _mm256_min_epi32(int3, max_mask);
            __m256i int_max4 = _mm256_min_epi32(int4, max_mask);

            // saturate at 0
            __m256i int_min1 = _mm256_max_epi32(int_max1, min_mask);
            __m256i int_min2 = _mm256_max_epi32(int_max2, min_mask);
            __m256i int_min3 = _mm256_max_epi32(int_max3, min_mask);
            __m256i int_min4 = _mm256_max_epi32(int_max4, min_mask);

            // reorganize bytes
            __m256i packed11 = _mm256_packs_epi32(int_min1, int_min1);
            r1.data = _mm256_packus_epi16(packed11, packed11);

            __m256i packed21 = _mm256_packs_epi32(int_min2, int_min2);
            r2.data = _mm256_packus_epi16(packed21, packed21);

            __m256i packed31 = _mm256_packs_epi32(int_min3, int_min3);
            r3.data = _mm256_packus_epi16(packed31, packed31);

            __m256i packed41 = _mm256_packs_epi32(int_min4, int_min4);
            r4.data = _mm256_packus_epi16(packed41, packed41);

            // build result
            r.ints[0] = r2.ints[0];
            r.ints[1] = r2.ints[4];

            r.ints[2] = r4.ints[0];
            r.ints[3] = r4.ints[4];

            r.ints[4] = r1.ints[0];
            r.ints[5] = r1.ints[4];

            r.ints[6] = r3.ints[0];
            r.ints[7] = r3.ints[4];

            // write result to memory
            _mm256_storeu_si256((__m256i *)(dst_line + x), r.data);
        }

        for (std::size_t x = to_x - (to_x % 32); x <= to_x; ++x)
        {
            std::int16_t v = static_cast<std::int16_t>(src1_line[x]) - static_cast<std::int16_t>(src2_line[x]) + static_cast<std::int16_t>(offset);

            dst_line[x] = static_cast<std::uint8_t>(std::max(static_cast<std::int16_t>(0), std::min(static_cast<std::int16_t>(255), v)));
        }
    }
}

#else

void diff_gray_8bit_noop(std::uint8_t * src1, std::uint8_t * src2, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
{
    const std::size_t image_width = parameters.image_width;

    const std::int32_t offset = parameters.signed_integer_numbers.at(0);

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

#endif

}

namespace cvpg { namespace imageproc { namespace algorithms {

void diff_gray_8bit(std::uint8_t * src1, std::uint8_t * src2, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
{
#if defined __AVX2__

    diff_gray_8bit_avx2(src1, src2, dst, from_x, to_x, from_y, to_y, std::move(parameters));

#else

    diff_gray_8bit_noop(src1, src2, dst, from_x, to_x, from_y, to_y, std::move(parameters));

#endif
}

}}} // namespace cvpg::imageproc::algorithms
