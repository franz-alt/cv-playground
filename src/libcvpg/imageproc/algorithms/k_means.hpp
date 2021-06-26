// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_K_MEANS_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_K_MEANS_HPP

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/image.hpp>

namespace cvpg::imageproc::algorithms {

boost::asynchronous::detail::callback_continuation<image_gray_8bit> k_means(image_gray_8bit image, std::size_t k, std::size_t max_iterations, std::uint8_t eps);

boost::asynchronous::detail::callback_continuation<image_rgb_8bit> k_means(image_rgb_8bit image, std::size_t k, std::size_t max_iterations, std::uint8_t eps);

} // namespace cvpg::imageproc::algoritms

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_K_MEANS_HPP
