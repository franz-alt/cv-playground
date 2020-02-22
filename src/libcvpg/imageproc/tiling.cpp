#include <libcvpg/imageproc/tiling.hpp>

#include <memory>

#include <libcvpg/imageproc/tiling/diff.hpp>
#include <libcvpg/imageproc/tiling/mean.hpp>
#include <libcvpg/imageproc/tiling/multiply_add.hpp>

namespace {

template<class pixel>
struct horizontal_tiling_task : public boost::asynchronous::continuation_task<void>
{
    horizontal_tiling_task(std::shared_ptr<pixel> src1, std::shared_ptr<pixel> src2, std::shared_ptr<pixel> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::tiling_params params)
        : boost::asynchronous::continuation_task<void>("horizontal_tiling_task")
        , m_src1(src1)
        , m_src2(src2)
        , m_dst(dst)
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
                case cvpg::imageproc::tiling_algorithms::diff:
                {
                    cvpg::imageproc::diff_gray_8bit(m_src1.get(), m_src2.get(), m_dst.get(), m_from_x, m_to_x, m_from_y, m_to_y, std::move(m_params));
                    break;
                }

                case cvpg::imageproc::tiling_algorithms::mean:
                {
                    cvpg::imageproc::mean_gray_8bit(m_src1.get(), m_dst.get(), m_from_x, m_to_x, m_from_y, m_to_y, std::move(m_params));
                    break;
                }

                case cvpg::imageproc::tiling_algorithms::multiply_add:
                {
                    cvpg::imageproc::multiply_add_gray_8bit(m_src1.get(), m_dst.get(), m_from_x, m_to_x, m_from_y, m_to_y, std::move(m_params));
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
                horizontal_tiling_task<pixel>(m_src1, m_src2, m_dst, m_from_x, half - 1, m_from_y, m_to_y, m_params),
                horizontal_tiling_task<pixel>(m_src1, m_src2, m_dst, half, m_to_x, m_from_y, m_to_y, m_params)
            );
        }
    }

private:
    std::shared_ptr<pixel> m_src1;
    std::shared_ptr<pixel> m_src2;

    std::shared_ptr<pixel> m_dst;

    std::size_t m_from_x;
    std::size_t m_to_x;
    std::size_t m_from_y;
    std::size_t m_to_y;

    cvpg::imageproc::tiling_params m_params;
};

template<class pixel>
struct vertical_tiling_task : public boost::asynchronous::continuation_task<void>
{
    vertical_tiling_task(std::shared_ptr<pixel> src1, std::shared_ptr<pixel> src2, std::shared_ptr<pixel> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::tiling_params params)
        : boost::asynchronous::continuation_task<void>("vertical_tiling_task")
        , m_src1(src1)
        , m_src2(src2)
        , m_dst(dst)
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
                horizontal_tiling_task<cvpg::image_gray_8bit::pixel_type>(m_src1, m_src2, m_dst, m_from_x, m_to_x, m_from_y, m_to_y, std::move(m_params))
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
                vertical_tiling_task<pixel>(m_src1, m_src2, m_dst, m_from_x, m_to_x, m_from_y, half - 1, m_params),
                vertical_tiling_task<pixel>(m_src1, m_src2, m_dst, m_from_x, m_to_x, half, m_to_y, m_params)
            );
        }
    }

private:
    std::shared_ptr<pixel> m_src1;
    std::shared_ptr<pixel> m_src2;

    std::shared_ptr<pixel> m_dst;

    std::size_t m_from_x;
    std::size_t m_to_x;
    std::size_t m_from_y;
    std::size_t m_to_y;

    cvpg::imageproc::tiling_params m_params;
};

struct tiling_gray_8bit_task : public boost::asynchronous::continuation_task<cvpg::image_gray_8bit>
{
    tiling_gray_8bit_task(cvpg::image_gray_8bit image, cvpg::imageproc::tiling_params params)
        : boost::asynchronous::continuation_task<cvpg::image_gray_8bit>("tiling_gray_8bit")
        , m_image1(image)
        , m_image2()
        , m_output(image.width(), image.height(), image.padding())
        , m_params(std::move(params))
    {
        m_params.image_width = image.width() + image.padding();
        m_params.image_height = image.height();
    }

    tiling_gray_8bit_task(cvpg::image_gray_8bit image1, cvpg::image_gray_8bit image2, cvpg::imageproc::tiling_params params)
        : boost::asynchronous::continuation_task<cvpg::image_gray_8bit>("tiling_gray_8bit")
        , m_image1(image1)
        , m_image2(image2)
        , m_output(image1.width(), image1.height(), image1.padding())
        , m_params(std::move(params))
    {
        if (m_image1.width() != m_image2.width() || m_image1.height() != m_image2.height())
        {
            // TODO throw exception
        }

        m_params.image_width = image1.width() + image1.padding();
        m_params.image_height = image1.height();
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
            m_image2.height() != 0 ? vertical_tiling_task<cvpg::image_gray_8bit::pixel_type>(m_image1.data(0), m_image2.data(0), m_output.data(0), 0, m_image1.width() - 1, 0, m_image1.height() - 1, m_params)
                                   : vertical_tiling_task<cvpg::image_gray_8bit::pixel_type>(m_image1.data(0), std::shared_ptr<std::uint8_t>(), m_output.data(0), 0, m_image1.width() - 1, 0, m_image1.height() - 1, m_params)
        );
    }

private:
    cvpg::image_gray_8bit m_image1;
    cvpg::image_gray_8bit m_image2;

    cvpg::image_gray_8bit m_output;

    cvpg::imageproc::tiling_params m_params;
};

struct tiling_rgb_8bit_task : public boost::asynchronous::continuation_task<cvpg::image_rgb_8bit>
{
    tiling_rgb_8bit_task(cvpg::image_rgb_8bit image, cvpg::imageproc::tiling_params params)
        : boost::asynchronous::continuation_task<cvpg::image_rgb_8bit>("tiling_rgb_8bit")
        , m_image1(std::move(image))
        , m_image2()
        , m_output(image.width(), image.height(), image.padding())
        , m_params(std::move(params))
    {}

    tiling_rgb_8bit_task(cvpg::image_rgb_8bit image1, cvpg::image_rgb_8bit image2, cvpg::imageproc::tiling_params params)
        : boost::asynchronous::continuation_task<cvpg::image_rgb_8bit>("tiling_gray_8bit")
        , m_image1(image1)
        , m_image2(image2)
        , m_output(image1.width(), image1.height(), image1.padding())
        , m_params(std::move(params))
    {
        if (m_image1.width() != m_image2.width() || m_image1.height() != m_image2.height())
        {
            // TODO throw exception
        }

        m_params.image_width = image1.width() + image1.padding();
        m_params.image_height = image1.height();
    }

    void operator()()
    {
        // TODO implement me
    }

private:
    cvpg::image_rgb_8bit m_image1;
    cvpg::image_rgb_8bit m_image2;

    cvpg::image_rgb_8bit m_output;

    cvpg::imageproc::tiling_params m_params;
};

}

namespace cvpg { namespace imageproc {

boost::asynchronous::detail::callback_continuation<image_gray_8bit> tiling(image_gray_8bit image, tiling_params params)
{
    return boost::asynchronous::top_level_callback_continuation<image_gray_8bit>(
               tiling_gray_8bit_task(std::move(image), std::move(params))
           );
}

boost::asynchronous::detail::callback_continuation<image_gray_8bit> tiling(image_gray_8bit image1, image_gray_8bit image2, tiling_params params)
{
    return boost::asynchronous::top_level_callback_continuation<image_gray_8bit>(
               tiling_gray_8bit_task(std::move(image1), std::move(image2), std::move(params))
           );
}

boost::asynchronous::detail::callback_continuation<image_rgb_8bit> tiling(image_rgb_8bit image, tiling_params params)
{
    return boost::asynchronous::top_level_callback_continuation<image_rgb_8bit>(
               tiling_rgb_8bit_task(std::move(image), std::move(params))
           );
}

boost::asynchronous::detail::callback_continuation<image_rgb_8bit> tiling(image_rgb_8bit image1, image_rgb_8bit image2, tiling_params params)
{
    return boost::asynchronous::top_level_callback_continuation<image_rgb_8bit>(
               tiling_rgb_8bit_task(std::move(image1), std::move(image2), std::move(params))
           );
}

}} // namespace cvpg::imageproc
