// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_HPP

#include <cstdint>
#include <memory>
#include <tuple>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/imageproc/algorithms/tiling/functors/image.hpp>

namespace cvpg { namespace imageproc { namespace algorithms {

namespace detail {

template<class input_type, class result_type>
struct horizontal_tiling_task : public boost::asynchronous::continuation_task<void>
{
    horizontal_tiling_task(std::shared_ptr<input_type> src1,
                           std::shared_ptr<input_type> src2,
                           std::shared_ptr<result_type> dst,
                           std::size_t from_x,
                           std::size_t to_x,
                           std::size_t from_y,
                           std::size_t to_y,
                           cvpg::imageproc::algorithms::tiling_parameters parameters,
                           std::function<std::tuple<std::shared_ptr<result_type>, std::shared_ptr<result_type> >(std::shared_ptr<result_type> dst)> create_intermediate_outputs_func,
                           std::function<void(std::shared_ptr<input_type> src1, std::shared_ptr<input_type> src2, std::shared_ptr<result_type> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)> tile_algorithm_task,
                           std::function<boost::asynchronous::detail::callback_continuation<std::shared_ptr<result_type> >(std::shared_ptr<result_type> dst1, std::shared_ptr<result_type> dst2, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)> horizontal_merge_task)
        : boost::asynchronous::continuation_task<void>("horizontal_tiling_task")
        , m_src1(std::move(src1))
        , m_src2(std::move(src2))
        , m_dst(std::move(dst))
        , m_from_x(from_x)
        , m_to_x(to_x)
        , m_from_y(from_y)
        , m_to_y(to_y)
        , m_parameters(std::move(parameters))
        , m_create_intermediate_outputs_func(std::move(create_intermediate_outputs_func))
        , m_tile_algorithm_task(std::move(tile_algorithm_task))
        , m_horizontal_merge_task(std::move(horizontal_merge_task))
    {}

    void operator()()
    {
        const std::size_t distance = m_to_x - m_from_x;

        if (distance < m_parameters.cutoff_x)
        {
            if (m_tile_algorithm_task)
            {
                m_tile_algorithm_task(m_src1, m_src2, m_dst, m_from_x, m_to_x, m_from_y, m_to_y, std::move(m_parameters));

                this_task_result().set_value();
            }
            else
            {
                // TODO error handling
            }
        }
        else
        {
            const std::size_t half = m_from_x + distance / 2;
           
            auto [ dst1, dst2 ] = m_create_intermediate_outputs_func(m_dst);

            boost::asynchronous::create_callback_continuation(
                [task_res = this->this_task_result()
                ,src1 = m_src1
                ,src2 = m_src2
                ,dst = m_dst
                ,dst1 = dst1
                ,dst2 = dst2
                ,from_x = m_from_x
                ,to_x = m_to_x
                ,from_y = m_from_y
                ,to_y = m_to_y
                ,parameters = m_parameters
                ,horizontal_merge_task = m_horizontal_merge_task](auto cont_res) mutable
                {
                    try
                    {
                        std::get<0>(cont_res).get();
                        std::get<1>(cont_res).get();

                        if (horizontal_merge_task)
                        {
                            boost::asynchronous::create_callback_continuation(
                                [task_res = std::move(task_res), dst](auto cont_res)
                                {
                                    try
                                    {
                                        auto merged = std::move(std::get<0>(cont_res).get());

                                        *dst = std::move(*merged);

                                        task_res.set_value();
                                    }
                                    catch (...)
                                    {
                                        task_res.set_exception(std::current_exception());
                                    }
                                },
                                horizontal_merge_task(dst1, dst2, from_x, to_x, from_y, to_y, std::move(parameters))
                            );
                        }
                        else
                        {
                            task_res.set_value();
                        }
                    }
                    catch (...)
                    {
                        task_res.set_exception(std::current_exception());
                    }
                },
                horizontal_tiling_task<input_type, result_type>(
                    m_src1,
                    m_src2,
                    dst1,
                    m_from_x,
                    half - 1,
                    m_from_y,
                    m_to_y,
                    m_parameters,
                    m_create_intermediate_outputs_func,
                    m_tile_algorithm_task,
                    m_horizontal_merge_task
                ),
                horizontal_tiling_task<input_type, result_type>(
                    m_src1,
                    m_src2,
                    dst2,
                    half,
                    m_to_x,
                    m_from_y,
                    m_to_y,
                    m_parameters,
                    m_create_intermediate_outputs_func,
                    m_tile_algorithm_task,
                    m_horizontal_merge_task
                )
            );
        }
    }

private:
    std::shared_ptr<input_type> m_src1;
    std::shared_ptr<input_type> m_src2;

    std::shared_ptr<result_type> m_dst;

    std::size_t m_from_x;
    std::size_t m_to_x;
    std::size_t m_from_y;
    std::size_t m_to_y;

    cvpg::imageproc::algorithms::tiling_parameters m_parameters;

    std::function<std::tuple<std::shared_ptr<result_type>, std::shared_ptr<result_type> >(std::shared_ptr<result_type> dst)> m_create_intermediate_outputs_func;

    std::function<void(std::shared_ptr<input_type> src1, std::shared_ptr<input_type> src2, std::shared_ptr<result_type> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)> m_tile_algorithm_task;
    std::function<boost::asynchronous::detail::callback_continuation<std::shared_ptr<result_type> >(std::shared_ptr<result_type> dst1, std::shared_ptr<result_type> dst2, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)> m_horizontal_merge_task;
};

template<class input_type, class result_type>
struct vertical_tiling_task : public boost::asynchronous::continuation_task<void>
{
    vertical_tiling_task(std::shared_ptr<input_type> src1,
                         std::shared_ptr<input_type> src2,
                         std::shared_ptr<result_type> dst,
                         std::size_t from_x,
                         std::size_t to_x,
                         std::size_t from_y,
                         std::size_t to_y,
                         cvpg::imageproc::algorithms::tiling_parameters parameters,
                         std::function<std::tuple<std::shared_ptr<result_type>, std::shared_ptr<result_type> >(std::shared_ptr<result_type> dst)> create_intermediate_outputs_fct,
                         std::function<void(std::shared_ptr<input_type> src1, std::shared_ptr<input_type> src2, std::shared_ptr<result_type> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)> tile_algorithm_task,
                         std::function<boost::asynchronous::detail::callback_continuation<std::shared_ptr<result_type> >(std::shared_ptr<result_type> dst1, std::shared_ptr<result_type> dst2, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)> horizontal_merge_task,
                         std::function<boost::asynchronous::detail::callback_continuation<std::shared_ptr<result_type> >(std::shared_ptr<result_type> dst1, std::shared_ptr<result_type> dst2, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)> vertical_merge_task)
        : boost::asynchronous::continuation_task<void>("vertical_tiling_task")
        , m_src1(std::move(src1))
        , m_src2(std::move(src2))
        , m_dst(std::move(dst))
        , m_from_x(from_x)
        , m_to_x(to_x)
        , m_from_y(from_y)
        , m_to_y(to_y)
        , m_parameters(std::move(parameters))
        , m_create_intermediate_outputs_func(std::move(create_intermediate_outputs_fct))
        , m_tile_algorithm_task(std::move(tile_algorithm_task))
        , m_horizontal_merge_task(std::move(horizontal_merge_task))
        , m_vertical_merge_task(std::move(vertical_merge_task))
    {}

    void operator()()
    {
        const std::size_t distance = m_to_y - m_from_y;

        if (distance < m_parameters.cutoff_y)
        {
            boost::asynchronous::create_callback_continuation(
                [task_res = this->this_task_result()
                ,src1 = m_src1
                ,src2 = m_src2
                ,dst = m_dst](auto cont_res) mutable
                {
                    try
                    {
                        std::get<0>(cont_res).get();

                        task_res.set_value();
                    }
                    catch (...)
                    {
                        task_res.set_exception(std::current_exception());
                    }
                },
                horizontal_tiling_task<input_type, result_type>(
                    std::move(m_src1),
                    std::move(m_src2),
                    std::move(m_dst),
                    m_from_x,
                    m_to_x,
                    m_from_y,
                    m_to_y,
                    std::move(m_parameters),
                    std::move(m_create_intermediate_outputs_func),
                    std::move(m_tile_algorithm_task),
                    std::move(m_horizontal_merge_task)
                )
            );
        }
        else
        {
            const std::size_t half = m_from_y + distance / 2;

            auto [ dst1, dst2 ] = m_create_intermediate_outputs_func(m_dst);

            boost::asynchronous::create_callback_continuation(
                [task_res = this->this_task_result()
                ,src1 = m_src1
                ,src2 = m_src2
                ,dst = m_dst
                ,dst1 = dst1
                ,dst2 = dst2
                ,from_x = m_from_x
                ,to_x = m_to_x
                ,from_y = m_from_y
                ,to_y = m_from_y
                ,parameters = m_parameters
                ,vertical_merge_task = m_vertical_merge_task](auto cont_res) mutable
                {
                    try
                    {
                        std::get<0>(cont_res).get();
                        std::get<1>(cont_res).get();

                        if (vertical_merge_task)
                        {
                            boost::asynchronous::create_callback_continuation(
                                [task_res = std::move(task_res), dst](auto cont_res)
                                {
                                    try
                                    {
                                        *dst = std::move(*std::get<0>(cont_res).get());

                                        task_res.set_value();
                                    }
                                    catch (...)
                                    {
                                        task_res.set_exception(std::current_exception());
                                    }
                                },
                                vertical_merge_task(dst1, dst2, from_x, to_x, from_y, to_y, std::move(parameters))
                            );
                        }
                        else
                        {
                            task_res.set_value();
                        }
                    }
                    catch (...)
                    {
                        task_res.set_exception(std::current_exception());
                    }
                },
                vertical_tiling_task<input_type, result_type>(
                    m_src1,
                    m_src2,
                    dst1,
                    m_from_x,
                    m_to_x,
                    m_from_y,
                    half - 1,
                    m_parameters,
                    m_create_intermediate_outputs_func,
                    m_tile_algorithm_task,
                    m_horizontal_merge_task,
                    m_vertical_merge_task
                ),
                vertical_tiling_task<input_type, result_type>(
                    m_src1,
                    m_src2,
                    dst2,
                    m_from_x,
                    m_to_x,
                    half,
                    m_to_y,
                    m_parameters,
                    m_create_intermediate_outputs_func,
                    m_tile_algorithm_task,
                    m_horizontal_merge_task,
                    m_vertical_merge_task
                )
            );
        }
    }

private:
    std::shared_ptr<input_type> m_src1;
    std::shared_ptr<input_type> m_src2;

    std::shared_ptr<result_type> m_dst;

    std::size_t m_from_x;
    std::size_t m_to_x;
    std::size_t m_from_y;
    std::size_t m_to_y;

    cvpg::imageproc::algorithms::tiling_parameters m_parameters;

    std::function<std::tuple<std::shared_ptr<result_type>, std::shared_ptr<result_type> >(std::shared_ptr<result_type> dst)> m_create_intermediate_outputs_func;

    std::function<void(std::shared_ptr<input_type> src1, std::shared_ptr<input_type> src2, std::shared_ptr<result_type> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)> m_tile_algorithm_task;

    std::function<boost::asynchronous::detail::callback_continuation<std::shared_ptr<result_type> >(std::shared_ptr<result_type> dst1, std::shared_ptr<result_type> dst2, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)> m_horizontal_merge_task;
    std::function<boost::asynchronous::detail::callback_continuation<std::shared_ptr<result_type> >(std::shared_ptr<result_type> dst1, std::shared_ptr<result_type> dst2, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)> m_vertical_merge_task;
};

template<class functor>
struct tiling_task : public boost::asynchronous::continuation_task<typename functor::result_type>
{
    tiling_task(functor && func)
        : boost::asynchronous::continuation_task<typename functor::result_type>("tiling_task")
        , m_func(std::forward<functor>(func))
    {}

    void operator()()
    {
        using image_type = typename functor::input_type;
        using result_type = typename functor::result_type;

        std::shared_ptr<image_type> image1;
        std::shared_ptr<image_type> image2;

        if (m_func.inputs.size() == 1)
        {
            image1 = std::make_shared<image_type>(std::move(m_func.inputs.at(0)));
        }
        else if (m_func.inputs.size() == 2)
        {
            image1 = std::make_shared<image_type>(std::move(m_func.inputs.at(0)));
            image2 = std::make_shared<image_type>(std::move(m_func.inputs.at(1)));
        }
        else
        {
            // TODO implement me!
        }

        auto output =
            (m_func.parameters.dst_image_width == 0 && m_func.parameters.dst_image_height == 0) ?
            std::make_shared<result_type>(std::move(m_func.create_output(image1->width(), image1->height()))) :
            std::make_shared<result_type>(std::move(m_func.create_output(m_func.parameters.dst_image_width, m_func.parameters.dst_image_height)));

        auto create_intermediate_outputs_fct = m_func.create_intermediate_outputs;

        boost::asynchronous::create_callback_continuation(
            [task_res = this->this_task_result(), image1, image2, output](auto cont_res) mutable
            {
                try
                {
                    std::get<0>(cont_res).get();

                    task_res.set_value(std::move(*output));
                }
                catch (...)
                {
                    task_res.set_exception(std::current_exception());
                }
            },
            vertical_tiling_task<image_type, result_type>(
                image1,
                image2,
                output,
                0,
                image1->width() - 1,
                0,
                image1->height() - 1,
                std::move(m_func.parameters),
                std::move(create_intermediate_outputs_fct),
                std::move(m_func.tile_algorithm_task),
                std::move(m_func.horizontal_merge_task),
                std::move(m_func.vertical_merge_task)
            )
        );
    }

private:
    functor m_func;
};

} // namespace detail

template<class functor>
boost::asynchronous::detail::callback_continuation<typename functor::result_type> tiling(functor && func)
{
    return boost::asynchronous::top_level_callback_continuation<typename functor::result_type>(
               detail::tiling_task<functor>(std::forward<functor>(func))
           );
}

}}} // namespace cvpg::imageproc::algoritms

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_HPP
