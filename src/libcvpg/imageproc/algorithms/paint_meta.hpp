#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_PAINT_META_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_PAINT_META_HPP

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/image.hpp>

namespace cvpg::imageproc::algorithms {

boost::asynchronous::detail::callback_continuation<image_gray_8bit> paint_meta(image_gray_8bit image, std::string key, std::string scores, std::string mode);

boost::asynchronous::detail::callback_continuation<image_rgb_8bit> paint_meta(image_rgb_8bit image, std::string key, std::string scores, std::string mode);

} // namespace cvpg::imageproc::algorithms

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_PAINT_META_HPP
