// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_HISTOGRAM_EQUALIZAtION_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_HISTOGRAM_EQUALIZAtION_HPP

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/image.hpp>

namespace cvpg::imageproc::algorithms {

boost::asynchronous::detail::callback_continuation<image_gray_8bit> histogram_equalization(image_gray_8bit image);

} // namespace cvpg::imageproc::algoritms

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_HISTOGRAM_EQUALIZAtION_HPP
