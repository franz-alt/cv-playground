#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_PARAMETERS_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_PARAMETERS_HPP

#include <cstdint>
#include <vector>

#include <libcvpg/imageproc/algorithms/border_mode.hpp>

namespace cvpg { namespace imageproc { namespace algorithms {

struct tiling_parameters
{
    std::size_t image_width = 0;
    std::size_t image_height = 0;

    std::size_t cutoff_x = 0; // horizontal cutoff
    std::size_t cutoff_y = 0; // vertical cutoff

    std::vector<double> real_numbers;
    std::vector<std::int32_t> signed_integer_numbers;

    cvpg::imageproc::algorithms::border_mode border_mode = cvpg::imageproc::algorithms::border_mode::ignore;
};

}}} // namespace cvpg::imageproc::algoritms

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_PARAMETERS_HPP
