// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_CONVERT_TO_GRAY_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_CONVERT_TO_GRAY_HPP

#include <ostream>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/image.hpp>

namespace cvpg::imageproc::algorithms {

enum class rgb_conversion_mode
{
    use_red,
    use_green,
    use_blue,
    calc_average
};

std::ostream & operator<<(std::ostream & out, rgb_conversion_mode const & mode);

boost::asynchronous::detail::callback_continuation<image_gray_8bit> convert_to_gray(image_rgb_8bit image, rgb_conversion_mode mode);

} // namespace cvpg::imageproc::algoritms

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_CONVERT_TO_GRAY_HPP
