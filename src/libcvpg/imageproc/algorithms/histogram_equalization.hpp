#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_HISTOGRAM_EQUALIZAtION_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_HISTOGRAM_EQUALIZAtION_HPP

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/image.hpp>

namespace cvpg { namespace imageproc { namespace algorithms {

boost::asynchronous::detail::callback_continuation<image_gray_8bit> histogram_equalization(image_gray_8bit image);

}}} // namespace cvpg::imageproc::algoritms

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_HISTOGRAM_EQUALIZAtION_HPP
