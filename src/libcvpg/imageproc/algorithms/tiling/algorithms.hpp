#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_ALGORITHMS_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_ALGORITHMS_HPP

namespace cvpg { namespace imageproc { namespace algorithms {

enum class tiling_algorithms
{
    unknown,
    convert_to_gray,
    diff,
    mean,
    multiply_add
};

}}} // namespace cvpg::imageproc::algoritms

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_ALGORITHMS_HPP
