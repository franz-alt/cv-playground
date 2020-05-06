#include <libcvpg/imageproc/algorithms/tiling/multiply_add.hpp>

#if defined __AVX2__

#include <immintrin.h>

#endif

namespace {

#if defined __AVX2__

inline void multiply_add_gray_8bit_avx2(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
{
    const std::size_t image_width = parameters.image_width;

    const double factor = parameters.real_numbers.at(0);
    const std::int32_t offset = parameters.signed_integer_numbers.at(0);

    // set single precision multiplication factor
    __m256 factor_ = _mm256_set1_ps(static_cast<float>(factor));

    // set offset added after multiplication
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

    std::uint8_t * src_line = nullptr;
    std::uint8_t * dst_line = nullptr;

    for (std::size_t y = from_y; y <= to_y; ++y)
    {
        const std::size_t offset_y = image_width * y;

        src_line = src + offset_y;
        dst_line = dst + offset_y;

        for (std::size_t x = from_x; x <= to_x; x += 32)
        {
            // load data from (unaligned) memory into 256-bit register
            __m256i data = _mm256_loadu_si256((__m256i const *)(src_line + x));

            // extract two 128-bit from 256-bit register
            __m128i data_high = _mm256_extracti128_si256(data, 1);
            __m128i data_low  = _mm256_extracti128_si256(data, 0);

            // convert lower half of 128-bit registers from 8-bit values to 32-bit values
            __m256i converted_high = _mm256_cvtepu8_epi32(data_high);
            __m256i converted_low  = _mm256_cvtepu8_epi32(data_low);

            // shuffle data to perform next extract & conversion
            __m256i data_shuffled = _mm256_shuffle_epi8(data, _mm256_setr_epi8( 8,  9, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,
                                                                                8,  9, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0));

            // extract two 128-bit from 256-bit register
            __m128i data_high_ = _mm256_extracti128_si256(data_shuffled, 1);
            __m128i data_low_  = _mm256_extracti128_si256(data_shuffled, 0);

            // convert lower half of 128-bit registers from 8-bit values to 32-bit values
            __m256i converted_high_ = _mm256_cvtepu8_epi32(data_high_);
            __m256i converted_low_  = _mm256_cvtepu8_epi32(data_low_);

            // convert 32-bit integers into single precision floats
            __m256 floats1 = _mm256_cvtepi32_ps(converted_high);
            __m256 floats2 = _mm256_cvtepi32_ps(converted_low);
            __m256 floats3 = _mm256_cvtepi32_ps(converted_high_);
            __m256 floats4 = _mm256_cvtepi32_ps(converted_low_);

            // multiply floats with factor and add offset
            __m256 fma1 = _mm256_fmadd_ps(floats1, factor_, offset_);
            __m256 fma2 = _mm256_fmadd_ps(floats2, factor_, offset_);
            __m256 fma3 = _mm256_fmadd_ps(floats3, factor_, offset_);
            __m256 fma4 = _mm256_fmadd_ps(floats4, factor_, offset_);

            // convert floats back to 32-bit integers
            __m256i int1 = _mm256_cvtps_epi32(fma1);
            __m256i int2 = _mm256_cvtps_epi32(fma2);
            __m256i int3 = _mm256_cvtps_epi32(fma3);
            __m256i int4 = _mm256_cvtps_epi32(fma4);

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
            std::int16_t v = static_cast<std::int16_t>(src_line[x] * factor) + static_cast<std::int16_t>(offset);

            dst_line[x] = static_cast<std::uint8_t>(std::max(static_cast<std::int16_t>(0), std::min(static_cast<std::int16_t>(255), v)));
        }
    }
}

#else

inline void multiply_add_gray_8bit_noop(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
{
    const std::size_t image_width = parameters.image_width;

    const double factor = parameters.real_numbers.at(0);
    const std::int32_t offset = parameters.signed_integer_numbers.at(0);

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

#endif

}

namespace cvpg { namespace imageproc { namespace algorithms {

void multiply_add_gray_8bit(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
{
#if defined __AVX2__

    multiply_add_gray_8bit_avx2(src, dst, from_x, to_x, from_y, to_y, std::move(parameters));

#else

    multiply_add_gray_8bit_noop(src, dst, from_x, to_x, from_y, to_y, std::move(parameters));

#endif
}

}}} // namespace cvpg::imageproc::algorithms
