#include <libcvpg/imageproc/algorithms/tiling.hpp>

#include <memory>

#include <libcvpg/imageproc/algorithms/tiling/convert_to_gray.hpp>
#include <libcvpg/imageproc/algorithms/tiling/diff.hpp>
#include <libcvpg/imageproc/algorithms/tiling/mean.hpp>
#include <libcvpg/imageproc/algorithms/tiling/multiply_add.hpp>

namespace {

template<class image, std::uint8_t channels>
struct horizontal_tiling_task : public boost::asynchronous::continuation_task<void> {};

template<class image>
struct horizontal_tiling_task<image, 1> : public boost::asynchronous::continuation_task<void>
{
    horizontal_tiling_task(image src1, image src2, image dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_params params)
        : boost::asynchronous::continuation_task<void>("horizontal_tiling_task")
        , m_src1(std::move(src1))
        , m_src2(std::move(src2))
        , m_dst(std::move(dst))
        , m_from_x(from_x)
        , m_to_x(to_x)
        , m_from_y(from_y)
        , m_to_y(to_y)
        , m_params(std::move(params))
    {}

    void operator()()
    {
        std::size_t distance = m_to_x - m_from_x;

        if (distance < m_params.cutoff_x)
        {
            switch (m_params.algorithm)
            {
                case cvpg::imageproc::algorithms::tiling_algorithms::diff:
                {
                    cvpg::imageproc::algorithms::diff_gray_8bit(m_src1.data(0).get(), m_src2.data(0).get(), m_dst.data(0).get(), m_from_x, m_to_x, m_from_y, m_to_y, std::move(m_params));
                    break;
                }

                case cvpg::imageproc::algorithms::tiling_algorithms::mean:
                {
                    cvpg::imageproc::algorithms::mean_gray_8bit(m_src1.data(0).get(), m_dst.data(0).get(), m_from_x, m_to_x, m_from_y, m_to_y, std::move(m_params));
                    break;
                }

                case cvpg::imageproc::algorithms::tiling_algorithms::multiply_add:
                {
                    cvpg::imageproc::algorithms::multiply_add_gray_8bit(m_src1.data(0).get(), m_dst.data(0).get(), m_from_x, m_to_x, m_from_y, m_to_y, std::move(m_params));
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
                horizontal_tiling_task<image, 1>(m_src1, m_src2, m_dst, m_from_x, half - 1, m_from_y, m_to_y, m_params),
                horizontal_tiling_task<image, 1>(m_src1, m_src2, m_dst, half, m_to_x, m_from_y, m_to_y, m_params)
            );
        }
    }

private:
    image m_src1;
    image m_src2;

    image m_dst;

    std::size_t m_from_x;
    std::size_t m_to_x;
    std::size_t m_from_y;
    std::size_t m_to_y;

    cvpg::imageproc::algorithms::tiling_params m_params;
};

template<class image>
struct horizontal_tiling_task<image, 3> : public boost::asynchronous::continuation_task<void>
{
    horizontal_tiling_task(image src1, image src2, image dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_params params)
        : boost::asynchronous::continuation_task<void>("horizontal_tiling_task")
        , m_src1(std::move(src1))
        , m_src2(std::move(src2))
        , m_dst(std::move(dst))
        , m_from_x(from_x)
        , m_to_x(to_x)
        , m_from_y(from_y)
        , m_to_y(to_y)
        , m_params(std::move(params))
    {}

    void operator()()
    {
        std::size_t distance = m_to_x - m_from_x;

        if (distance < m_params.cutoff_x)
        {
            switch (m_params.algorithm)
            {
                case cvpg::imageproc::algorithms::tiling_algorithms::convert_to_gray:
                {
                    cvpg::imageproc::algorithms::convert_to_gray_8bit(m_src1.data(0).get(), m_src1.data(1).get(), m_src1.data(2).get(), m_dst.data(0).get(), m_from_x, m_to_x, m_from_y, m_to_y, std::move(m_params));
                    break;
                }

                case cvpg::imageproc::algorithms::tiling_algorithms::diff:
                {
                    cvpg::imageproc::algorithms::diff_gray_8bit(m_src1.data(0).get(), m_src2.data(0).get(), m_dst.data(0).get(), m_from_x, m_to_x, m_from_y, m_to_y, m_params);
                    cvpg::imageproc::algorithms::diff_gray_8bit(m_src1.data(1).get(), m_src2.data(1).get(), m_dst.data(1).get(), m_from_x, m_to_x, m_from_y, m_to_y, m_params);
                    cvpg::imageproc::algorithms::diff_gray_8bit(m_src1.data(2).get(), m_src2.data(2).get(), m_dst.data(2).get(), m_from_x, m_to_x, m_from_y, m_to_y, std::move(m_params));
                    break;
                }

                case cvpg::imageproc::algorithms::tiling_algorithms::mean:
                {
                    cvpg::imageproc::algorithms::mean_gray_8bit(m_src1.data(0).get(), m_dst.data(0).get(), m_from_x, m_to_x, m_from_y, m_to_y, m_params);
                    cvpg::imageproc::algorithms::mean_gray_8bit(m_src1.data(1).get(), m_dst.data(1).get(), m_from_x, m_to_x, m_from_y, m_to_y, m_params);
                    cvpg::imageproc::algorithms::mean_gray_8bit(m_src1.data(2).get(), m_dst.data(2).get(), m_from_x, m_to_x, m_from_y, m_to_y, std::move(m_params));
                    break;
                }

                case cvpg::imageproc::algorithms::tiling_algorithms::multiply_add:
                {
                    cvpg::imageproc::algorithms::multiply_add_gray_8bit(m_src1.data(0).get(), m_dst.data(0).get(), m_from_x, m_to_x, m_from_y, m_to_y, m_params);
                    cvpg::imageproc::algorithms::multiply_add_gray_8bit(m_src1.data(1).get(), m_dst.data(1).get(), m_from_x, m_to_x, m_from_y, m_to_y, m_params);
                    cvpg::imageproc::algorithms::multiply_add_gray_8bit(m_src1.data(2).get(), m_dst.data(2).get(), m_from_x, m_to_x, m_from_y, m_to_y, std::move(m_params));
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
                horizontal_tiling_task<image, 3>(m_src1, m_src2, m_dst, m_from_x, half - 1, m_from_y, m_to_y, m_params),
                horizontal_tiling_task<image, 3>(m_src1, m_src2, m_dst, half, m_to_x, m_from_y, m_to_y, m_params)
            );
        }
    }

private:
    image m_src1;
    image m_src2;

    image m_dst;

    std::size_t m_from_x;
    std::size_t m_to_x;
    std::size_t m_from_y;
    std::size_t m_to_y;

    cvpg::imageproc::algorithms::tiling_params m_params;
};

template<class image>
struct vertical_tiling_task : public boost::asynchronous::continuation_task<void>
{
    vertical_tiling_task(image src1, image src2, image dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_params params)
        : boost::asynchronous::continuation_task<void>("vertical_tiling_task")
        , m_src1(std::move(src1))
        , m_src2(std::move(src2))
        , m_dst(std::move(dst))
        , m_from_x(from_x)
        , m_to_x(to_x)
        , m_from_y(from_y)
        , m_to_y(to_y)
        , m_params(std::move(params))
    {}

    void operator()()
    {
        std::size_t distance = m_to_y - m_from_y;

        if (distance < m_params.cutoff_y)
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
                horizontal_tiling_task<image, image::channels_size>(std::move(m_src1), std::move(m_src2), std::move(m_dst), m_from_x, m_to_x, m_from_y, m_to_y, std::move(m_params))
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
                vertical_tiling_task<image>(m_src1, m_src2, m_dst, m_from_x, m_to_x, m_from_y, half - 1, m_params),
                vertical_tiling_task<image>(m_src1, m_src2, m_dst, m_from_x, m_to_x, half, m_to_y, m_params)
            );
        }
    }

private:
    image m_src1;
    image m_src2;

    image m_dst;

    std::size_t m_from_x;
    std::size_t m_to_x;
    std::size_t m_from_y;
    std::size_t m_to_y;

    cvpg::imageproc::algorithms::tiling_params m_params;
};

template<class image>
struct tiling_task : public boost::asynchronous::continuation_task<image>
{
    tiling_task(image img, cvpg::imageproc::algorithms::tiling_params params)
        : boost::asynchronous::continuation_task<image>("tiling_task")
        , m_image1(img)
        , m_image2()
        , m_output(img.width(), img.height(), img.padding())
        , m_params(std::move(params))
    {
        m_params.image_width = img.width() + img.padding();
        m_params.image_height = img.height();
    }

    tiling_task(image img1, image img2, cvpg::imageproc::algorithms::tiling_params params)
        : boost::asynchronous::continuation_task<image>("tiling_gray_8bit")
        , m_image1(img1)
        , m_image2(img2)
        , m_output(img1.width(), img1.height(), img1.padding())
        , m_params(std::move(params))
    {
        if (m_image1.width() != m_image2.width() || m_image1.height() != m_image2.height())
        {
            // TODO throw exception
        }

        m_params.image_width = img1.width() + img1.padding();
        m_params.image_height = img1.height();
    }

    void operator()()
    {
        boost::asynchronous::create_callback_continuation(
            [task_res = this->this_task_result(), image1 = m_image1, image2 = m_image2, output = m_output](auto cont_res) mutable
            {
                try
                {
                    std::get<0>(cont_res).get();

                    task_res.set_value(std::move(output));
                }
                catch (...)
                {
                    task_res.set_exception(std::current_exception());
                }
            },
            m_image2.height() != 0 ? vertical_tiling_task<image>(m_image1, m_image2, m_output, 0, m_image1.width() - 1, 0, m_image1.height() - 1, m_params)
                                   : vertical_tiling_task<image>(m_image1, image(), m_output, 0, m_image1.width() - 1, 0, m_image1.height() - 1, m_params)
        );
    }

private:
    image m_image1;
    image m_image2;

    image m_output;

    cvpg::imageproc::algorithms::tiling_params m_params;
};

}

namespace cvpg { namespace imageproc { namespace algorithms {

boost::asynchronous::detail::callback_continuation<image_gray_8bit> tiling(image_gray_8bit image, tiling_params params)
{
    return boost::asynchronous::top_level_callback_continuation<image_gray_8bit>(
               tiling_task(std::move(image), std::move(params))
           );
}

boost::asynchronous::detail::callback_continuation<image_gray_8bit> tiling(image_gray_8bit image1, image_gray_8bit image2, tiling_params params)
{
    return boost::asynchronous::top_level_callback_continuation<image_gray_8bit>(
               tiling_task(std::move(image1), std::move(image2), std::move(params))
           );
}

boost::asynchronous::detail::callback_continuation<image_rgb_8bit> tiling(image_rgb_8bit image, tiling_params params)
{
    return boost::asynchronous::top_level_callback_continuation<image_rgb_8bit>(
               tiling_task(std::move(image), std::move(params))
           );
}

boost::asynchronous::detail::callback_continuation<image_rgb_8bit> tiling(image_rgb_8bit image1, image_rgb_8bit image2, tiling_params params)
{
    return boost::asynchronous::top_level_callback_continuation<image_rgb_8bit>(
               tiling_task(std::move(image1), std::move(image2), std::move(params))
           );
}

}}} // namespace cvpg::imageproc::algorithms
