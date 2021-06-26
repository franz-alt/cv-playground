// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_FUNCTORS_HISTOGRAM_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_FUNCTORS_HISTOGRAM_HPP

#include <cstdint>
#include <functional>
#include <tuple>
#include <vector>

#include <libcvpg/imageproc/algorithms/tiling/parameters.hpp>

namespace cvpg { namespace imageproc { namespace algorithms { namespace tiling_functors {

//
// Functor to process a vector of images and return a histogram as a result.
//
template<class input_image, class result_histogram>
struct histogram
{
    using input_type = input_image;
    using result_type = result_histogram;

    std::vector<input_image> inputs;

    cvpg::imageproc::algorithms::tiling_parameters parameters;

    std::function<result_type(std::uint32_t width, std::uint32_t height)> create_output =
        [this](std::uint32_t /*width*/, std::uint32_t /*height*/)
        {
            return result_type();
        };

    std::function<std::tuple<std::shared_ptr<result_type>, std::shared_ptr<result_type> >(std::shared_ptr<result_type> dst)> create_intermediate_outputs =
        [](std::shared_ptr<result_type> dst)
        {
            auto dst1 = std::make_shared<result_type>();
            auto dst2 = std::make_shared<result_type>();

            return std::tuple<std::shared_ptr<result_type>, std::shared_ptr<result_type> >(dst1, dst2);
        };

    std::function<void(std::shared_ptr<input_type> src1, std::shared_ptr<input_type> src2, std::shared_ptr<result_type> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)> tile_algorithm_task;

    std::function<boost::asynchronous::detail::callback_continuation<std::shared_ptr<result_type> >(std::shared_ptr<result_type> dst1, std::shared_ptr<result_type> dst2, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)> horizontal_merge_task;
    std::function<boost::asynchronous::detail::callback_continuation<std::shared_ptr<result_type> >(std::shared_ptr<result_type> dst1, std::shared_ptr<result_type> dst2, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)> vertical_merge_task;
};

}}}} // namespace cvpg::imageproc::algoritms::tiling_functors

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_FUNCTORS_HISTOGRAM_HPP
