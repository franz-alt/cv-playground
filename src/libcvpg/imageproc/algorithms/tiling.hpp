#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_HPP

#include <cstdint>
#include <memory>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/imageproc/algorithms/tiling/functors/image.hpp>

namespace cvpg { namespace imageproc { namespace algorithms {

namespace detail {

template<class input_type, class result_type>
struct vertical_tiling_task : public boost::asynchronous::continuation_task<void>
{
    vertical_tiling_task(std::shared_ptr<input_type> src1, std::shared_ptr<input_type> src2, std::shared_ptr<result_type> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_algorithms algorithm, cvpg::imageproc::algorithms::tiling_parameters parameters)
        : boost::asynchronous::continuation_task<void>("vertical_tiling_task")
        , m_src1(std::move(src1))
        , m_src2(std::move(src2))
        , m_dst(std::move(dst))
        , m_from_x(from_x)
        , m_to_x(to_x)
        , m_from_y(from_y)
        , m_to_y(to_y)
        , m_algorithm(algorithm)
        , m_parameters(std::move(parameters))
    {}

    void operator()()
    {
        const std::size_t distance = m_to_y - m_from_y;

        if (distance < m_parameters.cutoff_y)
        {
            boost::asynchronous::create_callback_continuation(
                [task_res = this->this_task_result()](auto cont_res) mutable
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
                tiling_functors::horizontal_tiling_task<input_type, result_type>(std::move(m_src1), std::move(m_src2), std::move(m_dst), m_from_x, m_to_x, m_from_y, m_to_y, m_algorithm, std::move(m_parameters))
            );
        }
        else
        {
            std::size_t half = m_from_y + distance / 2;

            boost::asynchronous::create_callback_continuation(
                [task_res = this->this_task_result()](auto cont_res) mutable
                {
                    try
                    {
                        std::get<0>(cont_res).get();
                        std::get<1>(cont_res).get();

                        task_res.set_value();
                    }
                    catch (...)
                    {
                        task_res.set_exception(std::current_exception());
                    }
                },
                vertical_tiling_task<input_type, result_type>(m_src1, m_src2, m_dst, m_from_x, m_to_x, m_from_y, half - 1, m_algorithm, m_parameters),
                vertical_tiling_task<input_type, result_type>(m_src1, m_src2, m_dst, m_from_x, m_to_x, half, m_to_y, m_algorithm, m_parameters)
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

    cvpg::imageproc::algorithms::tiling_algorithms m_algorithm;

    cvpg::imageproc::algorithms::tiling_parameters m_parameters;
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

        auto output = std::make_shared<result_type>(image1->width(), image1->height());

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
            vertical_tiling_task<image_type, result_type>(image1, image2, output, 0, image1->width() - 1, 0, image1->height() - 1, m_func.algorithm, std::move(m_func.parameters))
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
