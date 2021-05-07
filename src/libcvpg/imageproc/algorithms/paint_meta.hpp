#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_PAINT_META_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_PAINT_META_HPP

#include <cstdint>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/image.hpp>

namespace cvpg::imageproc::algorithms {

boost::asynchronous::detail::callback_continuation<image_gray_8bit> paint_meta(image_gray_8bit image,
                                                                               std::string key,
                                                                               std::string mode,
                                                                               std::string classes_key,
                                                                               std::string classes_list,
                                                                               std::string scores_key,
                                                                               std::int32_t min_score,
                                                                               std::uint8_t gray);

boost::asynchronous::detail::callback_continuation<image_rgb_8bit> paint_meta(image_rgb_8bit image,
                                                                              std::string key,
                                                                              std::string mode,
                                                                              std::string classes_key,
                                                                              std::string classes_list,
                                                                              std::string scores_key,
                                                                              std::int32_t min_score,
                                                                              std::uint8_t red,
                                                                              std::uint8_t green,
                                                                              std::uint8_t blue);

} // namespace cvpg::imageproc::algorithms

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_PAINT_META_HPP
