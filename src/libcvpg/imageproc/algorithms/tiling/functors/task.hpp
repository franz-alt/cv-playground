#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_FUNCTORS_TASK_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_FUNCTORS_TASK_HPP

#include <boost/asynchronous/continuation_task.hpp>

namespace cvpg { namespace imageproc { namespace algorithms { namespace tiling_functors {

template<class input_type, class result_type>
struct horizontal_tiling_task : public boost::asynchronous::continuation_task<void> {};

}}}} // namespace cvpg::imageproc::algoritms::tiling_functors

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_FUNCTORS_TASK_HPP
