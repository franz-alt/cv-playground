#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_FUNCTORS_IMAGE_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_FUNCTORS_IMAGE_HPP

#include <cstdint>
#include <vector>

#include <libcvpg/imageproc/algorithms/tiling/algorithms.hpp>
#include <libcvpg/imageproc/algorithms/tiling/convert_to_gray.hpp>
#include <libcvpg/imageproc/algorithms/tiling/diff.hpp>
#include <libcvpg/imageproc/algorithms/tiling/mean.hpp>
#include <libcvpg/imageproc/algorithms/tiling/multiply_add.hpp>
#include <libcvpg/imageproc/algorithms/tiling/parameters.hpp>
#include <libcvpg/imageproc/algorithms/tiling/functors/task.hpp>

namespace cvpg { namespace imageproc { namespace algorithms { namespace tiling_functors {

//
// Functor to process a vector of images and return a single image as a result.
//
// Parameters are available as vectors of different types.
//
template<class input_image, class result_image = input_image>
struct image
{
    using input_type = input_image;
    using result_type = result_image;

    std::vector<input_image> inputs;

    cvpg::imageproc::algorithms::tiling_algorithms algorithm = cvpg::imageproc::algorithms::tiling_algorithms::unknown;
    
    cvpg::imageproc::algorithms::tiling_parameters parameters;
};

template<>
struct horizontal_tiling_task<cvpg::image_gray_8bit, cvpg::image_gray_8bit> : public boost::asynchronous::continuation_task<void>
{
    horizontal_tiling_task(std::shared_ptr<cvpg::image_gray_8bit> src1, std::shared_ptr<cvpg::image_gray_8bit> src2, std::shared_ptr<cvpg::image_gray_8bit> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_algorithms algorithm, cvpg::imageproc::algorithms::tiling_parameters parameters)
        : boost::asynchronous::continuation_task<void>("horizontal_tiling_task")
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
        const std::size_t distance = m_to_x - m_from_x;

        if (distance < m_parameters.cutoff_x)
        {
            switch (m_algorithm)
            {
                case cvpg::imageproc::algorithms::tiling_algorithms::diff:
                {
                    cvpg::imageproc::algorithms::diff_gray_8bit(m_src1->data(0).get(), m_src2->data(0).get(), m_dst->data(0).get(), m_from_x, m_to_x, m_from_y, m_to_y, std::move(m_parameters));
                    break;
                }

                case cvpg::imageproc::algorithms::tiling_algorithms::mean:
                {
                    cvpg::imageproc::algorithms::mean_gray_8bit(m_src1->data(0).get(), m_dst->data(0).get(), m_from_x, m_to_x, m_from_y, m_to_y, std::move(m_parameters));
                    break;
                }

                case cvpg::imageproc::algorithms::tiling_algorithms::multiply_add:
                {
                    cvpg::imageproc::algorithms::multiply_add_gray_8bit(m_src1->data(0).get(), m_dst->data(0).get(), m_from_x, m_to_x, m_from_y, m_to_y, std::move(m_parameters));
                    break;
                }

                default:
                {
                    // TODO error handling
                }
            }

            this->this_task_result().set_value();
        }
        else
        {
            std::size_t half = m_from_x + distance / 2;

            boost::asynchronous::create_callback_continuation(
                [task_res = this->this_task_result()](auto cont_res)
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
                horizontal_tiling_task<cvpg::image_gray_8bit, cvpg::image_gray_8bit>(m_src1, m_src2, m_dst, m_from_x, half - 1, m_from_y, m_to_y, m_algorithm, m_parameters),
                horizontal_tiling_task<cvpg::image_gray_8bit, cvpg::image_gray_8bit>(m_src1, m_src2, m_dst, half, m_to_x, m_from_y, m_to_y, m_algorithm, m_parameters)
            );
        }
    }

private:
    std::shared_ptr<cvpg::image_gray_8bit> m_src1;
    std::shared_ptr<cvpg::image_gray_8bit> m_src2;

    std::shared_ptr<cvpg::image_gray_8bit> m_dst;

    std::size_t m_from_x;
    std::size_t m_to_x;
    std::size_t m_from_y;
    std::size_t m_to_y;

    cvpg::imageproc::algorithms::tiling_algorithms m_algorithm;

    cvpg::imageproc::algorithms::tiling_parameters m_parameters;
};

template<>
struct horizontal_tiling_task<cvpg::image_rgb_8bit, cvpg::image_rgb_8bit> : public boost::asynchronous::continuation_task<void>
{
    horizontal_tiling_task(std::shared_ptr<cvpg::image_rgb_8bit> src1, std::shared_ptr<cvpg::image_rgb_8bit> src2, std::shared_ptr<cvpg::image_rgb_8bit> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_algorithms algorithm, cvpg::imageproc::algorithms::tiling_parameters parameters)
        : boost::asynchronous::continuation_task<void>("horizontal_tiling_task")
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
        const std::size_t distance = m_to_x - m_from_x;

        if (distance < m_parameters.cutoff_x)
        {
            switch (m_algorithm)
            {
                case cvpg::imageproc::algorithms::tiling_algorithms::diff:
                {
                    cvpg::imageproc::algorithms::diff_gray_8bit(m_src1->data(0).get(), m_src2->data(0).get(), m_dst->data(0).get(), m_from_x, m_to_x, m_from_y, m_to_y, m_parameters);
                    cvpg::imageproc::algorithms::diff_gray_8bit(m_src1->data(1).get(), m_src2->data(1).get(), m_dst->data(1).get(), m_from_x, m_to_x, m_from_y, m_to_y, m_parameters);
                    cvpg::imageproc::algorithms::diff_gray_8bit(m_src1->data(2).get(), m_src2->data(2).get(), m_dst->data(2).get(), m_from_x, m_to_x, m_from_y, m_to_y, std::move(m_parameters));
                    break;
                }

                case cvpg::imageproc::algorithms::tiling_algorithms::mean:
                {
                    cvpg::imageproc::algorithms::mean_gray_8bit(m_src1->data(0).get(), m_dst->data(0).get(), m_from_x, m_to_x, m_from_y, m_to_y, m_parameters);
                    cvpg::imageproc::algorithms::mean_gray_8bit(m_src1->data(1).get(), m_dst->data(1).get(), m_from_x, m_to_x, m_from_y, m_to_y, m_parameters);
                    cvpg::imageproc::algorithms::mean_gray_8bit(m_src1->data(2).get(), m_dst->data(2).get(), m_from_x, m_to_x, m_from_y, m_to_y, std::move(m_parameters));
                    break;
                }

                case cvpg::imageproc::algorithms::tiling_algorithms::multiply_add:
                {
                    cvpg::imageproc::algorithms::multiply_add_gray_8bit(m_src1->data(0).get(), m_dst->data(0).get(), m_from_x, m_to_x, m_from_y, m_to_y, m_parameters);
                    cvpg::imageproc::algorithms::multiply_add_gray_8bit(m_src1->data(1).get(), m_dst->data(1).get(), m_from_x, m_to_x, m_from_y, m_to_y, m_parameters);
                    cvpg::imageproc::algorithms::multiply_add_gray_8bit(m_src1->data(2).get(), m_dst->data(2).get(), m_from_x, m_to_x, m_from_y, m_to_y, std::move(m_parameters));
                    break;
                }

                default:
                {
                    // TODO error handling
                }
            }

            this->this_task_result().set_value();
        }
        else
        {
            std::size_t half = m_from_x + distance / 2;

            boost::asynchronous::create_callback_continuation(
                [task_res = this->this_task_result()](auto cont_res)
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
                horizontal_tiling_task<cvpg::image_rgb_8bit, cvpg::image_rgb_8bit>(m_src1, m_src2, m_dst, m_from_x, half - 1, m_from_y, m_to_y, m_algorithm, m_parameters),
                horizontal_tiling_task<cvpg::image_rgb_8bit, cvpg::image_rgb_8bit>(m_src1, m_src2, m_dst, half, m_to_x, m_from_y, m_to_y, m_algorithm, m_parameters)
            );
        }
    }

private:
    std::shared_ptr<cvpg::image_rgb_8bit> m_src1;
    std::shared_ptr<cvpg::image_rgb_8bit> m_src2;

    std::shared_ptr<cvpg::image_rgb_8bit> m_dst;

    std::size_t m_from_x;
    std::size_t m_to_x;
    std::size_t m_from_y;
    std::size_t m_to_y;

    cvpg::imageproc::algorithms::tiling_algorithms m_algorithm;

    cvpg::imageproc::algorithms::tiling_parameters m_parameters;
};

template<>
struct horizontal_tiling_task<cvpg::image_rgb_8bit, cvpg::image_gray_8bit> : public boost::asynchronous::continuation_task<void>
{
    horizontal_tiling_task(std::shared_ptr<cvpg::image_rgb_8bit> src1, std::shared_ptr<cvpg::image_rgb_8bit> src2, std::shared_ptr<cvpg::image_gray_8bit> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_algorithms algorithm, cvpg::imageproc::algorithms::tiling_parameters parameters)
        : boost::asynchronous::continuation_task<void>("horizontal_tiling_task")
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
        const std::size_t distance = m_to_x - m_from_x;

        if (distance < m_parameters.cutoff_x)
        {
            switch (m_algorithm)
            {
                case cvpg::imageproc::algorithms::tiling_algorithms::convert_to_gray:
                {
                    cvpg::imageproc::algorithms::convert_to_gray_8bit(m_src1->data(0).get(), m_src1->data(1).get(), m_src1->data(2).get(), m_dst->data(0).get(), m_from_x, m_to_x, m_from_y, m_to_y, std::move(m_parameters));
                    break;
                }

                default:
                {
                    // TODO error handling
                }
            }

            this->this_task_result().set_value();
        }
        else
        {
            std::size_t half = m_from_x + distance / 2;

            boost::asynchronous::create_callback_continuation(
                [task_res = this->this_task_result()](auto cont_res)
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
                horizontal_tiling_task<cvpg::image_rgb_8bit, cvpg::image_gray_8bit>(m_src1, m_src2, m_dst, m_from_x, half - 1, m_from_y, m_to_y, m_algorithm, m_parameters),
                horizontal_tiling_task<cvpg::image_rgb_8bit, cvpg::image_gray_8bit>(m_src1, m_src2, m_dst, half, m_to_x, m_from_y, m_to_y, m_algorithm, m_parameters)
            );
        }
    }

private:
    std::shared_ptr<cvpg::image_rgb_8bit> m_src1;
    std::shared_ptr<cvpg::image_rgb_8bit> m_src2;

    std::shared_ptr<cvpg::image_gray_8bit> m_dst;

    std::size_t m_from_x;
    std::size_t m_to_x;
    std::size_t m_from_y;
    std::size_t m_to_y;

    cvpg::imageproc::algorithms::tiling_algorithms m_algorithm;

    cvpg::imageproc::algorithms::tiling_parameters m_parameters;
};

}}}} // namespace cvpg::imageproc::algoritms::tiling_functors

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_TILING_FUNCTORS_IMAGE_HPP
