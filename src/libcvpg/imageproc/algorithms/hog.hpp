#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_HOG_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_HOG_HPP

#include <vector>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/image.hpp>
#include <libcvpg/core/histogram.hpp>

namespace cvpg::imageproc::algorithms {

boost::asynchronous::detail::callback_continuation<std::vector<histogram<double> > > hog(image_gray_8bit image, std::size_t cell_dimension = 8);

boost::asynchronous::detail::callback_continuation<std::vector<histogram<double> > > hog(image_rgb_8bit image, std::size_t cell_dimension = 8);

boost::asynchronous::detail::callback_continuation<image_gray_8bit> hog_image(std::vector<histogram<double> > histograms, std::size_t cells_per_row, std::size_t cell_dimension = 8);

boost::asynchronous::detail::callback_continuation<image_gray_8bit> hog_image(image_gray_8bit image);

boost::asynchronous::detail::callback_continuation<image_gray_8bit> hog_image(image_rgb_8bit image);

} // namespace cvpg::imageproc::algoritms

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_HOG_HPP
