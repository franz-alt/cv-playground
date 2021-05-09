#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_OTSU_THRESHOLD_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_OTSU_THRESHOLD_HPP

#include <cstdint>

#include <libcvpg/core/histogram.hpp>

namespace cvpg::imageproc::algorithms {

std::uint8_t otsu_threshold(cvpg::histogram<std::size_t> const & h);

} // namespace cvpg::imageproc::algoritms

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_OTSU_THRESHOLD_HPP
