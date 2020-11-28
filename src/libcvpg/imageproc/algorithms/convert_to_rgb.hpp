#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_CONVERT_TO_RGB_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_CONVERT_TO_RGB_HPP

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/image.hpp>

namespace cvpg { namespace imageproc { namespace algorithms {

boost::asynchronous::detail::callback_continuation<image_rgb_8bit> convert_to_rgb(image_gray_8bit image);

}}} // namespace cvpg::imageproc::algoritms

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_CONVERT_TO_RGB_HPP
