// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/imageproc/algorithms/histogram_equalization.hpp>

#include <memory>

#include <libcvpg/core/histogram.hpp>
#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/algorithms/tiling.hpp>
#include <libcvpg/imageproc/algorithms/tiling/histogram.hpp>
#include <libcvpg/imageproc/algorithms/tiling/functors/histogram.hpp>
#include <libcvpg/imageproc/algorithms/tiling/functors/image.hpp>

namespace {

void equalize_gray_8bit(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters, std::shared_ptr<const cvpg::histogram<std::size_t> > cdf, std::size_t min_cdf, std::size_t max_cdf)
{
    const std::size_t image_width = parameters.image_width;
    const std::size_t image_height = parameters.image_height;

    const std::size_t pixels = image_width * image_height;

    std::uint8_t * src_line = nullptr;
    std::uint8_t * dst_line = nullptr;

    for (std::int32_t y = from_y; y <= to_y; ++y)
    {
        const std::size_t offset_y = image_width * y;

        src_line = src + offset_y;
        dst_line = dst + offset_y;

        for (std::size_t x = from_x; x <= to_x; ++x)
        {
            dst_line[x] = static_cast<std::uint8_t>((static_cast<double>(cdf->at(src_line[x])) - min_cdf) / (pixels - min_cdf) * (256.0 - 1));
        }
    }
}

struct equalize_image_task : public boost::asynchronous::continuation_task<cvpg::image_gray_8bit>
{
    equalize_image_task(cvpg::image_gray_8bit image, cvpg::histogram<std::size_t> cdf, std::size_t min_cdf, std::size_t max_cdf)
        : boost::asynchronous::continuation_task<cvpg::image_gray_8bit>("equalize_image_task")
        , m_image(std::move(image))
        , m_cdf(std::make_shared<const cvpg::histogram<std::size_t> >(std::move(cdf)))
        , m_min_cdf(min_cdf)
        , m_max_cdf(max_cdf)
    {}

    void operator()()
    {
        const auto width = m_image.width();
        const auto height = m_image.height();

        auto tf = cvpg::imageproc::algorithms::tiling_functors::image<cvpg::image_gray_8bit, cvpg::image_gray_8bit>({{ std::move(m_image) }});
        tf.parameters.image_width = width;
        tf.parameters.image_height = height;
        tf.parameters.cutoff_x = 512;
        tf.parameters.cutoff_y = 512;

        tf.tile_algorithm_task = [cdf = m_cdf, min_cdf = m_min_cdf, max_cdf = m_max_cdf](std::shared_ptr<cvpg::image_gray_8bit> src1, std::shared_ptr<cvpg::image_gray_8bit> /*src2*/, std::shared_ptr<cvpg::image_gray_8bit> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
        {
            equalize_gray_8bit(src1->data(0).get(), dst->data(0).get(), from_x, to_x, from_y, to_y, std::move(parameters), cdf, min_cdf, max_cdf);
        };

        boost::asynchronous::create_callback_continuation(
            [result = this->this_task_result()](auto cont_res) mutable
            {
                try
                {
                    result.set_value(std::move(std::get<0>(cont_res).get()));
                }
                catch (...)
                {
                    result.set_exception(std::current_exception());
                }
            },
            cvpg::imageproc::algorithms::tiling(std::move(tf))
        );
    }

private:
    cvpg::image_gray_8bit m_image;

    std::shared_ptr<const cvpg::histogram<std::size_t> > m_cdf;

    std::size_t m_min_cdf;
    std::size_t m_max_cdf;
};

boost::asynchronous::detail::callback_continuation<cvpg::image_gray_8bit>
equalize_image(cvpg::image_gray_8bit image, cvpg::histogram<std::size_t> cdf, std::size_t min_cdf, std::size_t max_cdf)
{
    return boost::asynchronous::top_level_callback_continuation<cvpg::image_gray_8bit>(
               equalize_image_task(std::move(image), std::move(cdf), min_cdf, max_cdf)
           );
}

struct merge_histograms_task : public boost::asynchronous::continuation_task<std::shared_ptr<cvpg::histogram<std::size_t> > >
{
    merge_histograms_task(std::shared_ptr<cvpg::histogram<std::size_t> > a, std::shared_ptr<cvpg::histogram<std::size_t> > b)
        : boost::asynchronous::continuation_task<std::shared_ptr<cvpg::histogram<std::size_t> > >("merge_histograms")
        , m_a(a)
        , m_b(b)
    {}

    void operator()()
    {
        // TODO optimize this
        this_task_result().set_value(std::make_shared<cvpg::histogram<std::size_t> >(*m_a + *m_b));
    }

private:
    std::shared_ptr<cvpg::histogram<std::size_t> > m_a;
    std::shared_ptr<cvpg::histogram<std::size_t> > m_b;
};

boost::asynchronous::detail::callback_continuation<std::shared_ptr<cvpg::histogram<std::size_t> > >
merge_histograms(std::shared_ptr<cvpg::histogram<std::size_t> > a, std::shared_ptr<cvpg::histogram<std::size_t> > b)
{
    return boost::asynchronous::top_level_callback_continuation<std::shared_ptr<cvpg::histogram<std::size_t> > >(
               merge_histograms_task(a, b)
           );
}

struct histogram_equalization_task : public boost::asynchronous::continuation_task<cvpg::image_gray_8bit>
{
    histogram_equalization_task(cvpg::image_gray_8bit image)
        : boost::asynchronous::continuation_task<cvpg::image_gray_8bit>("histogram_equalization")
        , m_image(std::move(image))
    {}

    void operator()()
    {
        const auto width = m_image.width();
        const auto height = m_image.height();

        auto tf = cvpg::imageproc::algorithms::tiling_functors::histogram<cvpg::image_gray_8bit, cvpg::histogram<std::size_t> >({{ m_image }});
        tf.parameters.image_width = width;
        tf.parameters.image_height = height;
        tf.parameters.cutoff_x = 512;
        tf.parameters.cutoff_y = 512;

        tf.tile_algorithm_task = [](std::shared_ptr<cvpg::image_gray_8bit> src1, std::shared_ptr<cvpg::image_gray_8bit> /*src2*/, std::shared_ptr<cvpg::histogram<std::size_t> > dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
        {
            std::vector<std::size_t> h(256);
            cvpg::imageproc::algorithms::histogram_gray_8bit(src1->data(0).get(), &h, from_x, to_x, from_y, to_y, std::move(parameters));

            *dst = cvpg::histogram<std::size_t> (std::move(h));
        };

        tf.horizontal_merge_task = [](std::shared_ptr<cvpg::histogram<std::size_t> > dst1, std::shared_ptr<cvpg::histogram<std::size_t> > dst2, std::size_t /*from_x*/, std::size_t /*to_x*/, std::size_t /*from_y*/, std::size_t /*to_y*/, cvpg::imageproc::algorithms::tiling_parameters /*parameters*/)
        {
            return merge_histograms(dst1, dst2);
        };

        tf.vertical_merge_task = [](std::shared_ptr<cvpg::histogram<std::size_t> > dst1, std::shared_ptr<cvpg::histogram<std::size_t> > dst2, std::size_t /*from_x*/, std::size_t /*to_x*/, std::size_t /*from_y*/, std::size_t /*to_y*/, cvpg::imageproc::algorithms::tiling_parameters /*parameters*/)
        {
            return merge_histograms(dst1, dst2);
        };

        boost::asynchronous::create_callback_continuation(
            [result = this->this_task_result(), image = std::move(m_image)](auto cont_res)
            {
                auto histogram = std::move(std::get<0>(cont_res).get());

                // calculate cumulative distribution function (cdf)
                std::size_t counter = 0;
                std::size_t min_cdf = -1;
                std::size_t max_cdf = -1;

                std::vector<std::size_t> cdf(histogram.bins());

                for (std::size_t i = 0; i < histogram.bins(); ++i)
                {
                    if (histogram.at(i) != 0)
                    {
                        if (min_cdf == -1)
                        {
                            min_cdf = i;
                        }

                        max_cdf = i;

                        counter += histogram.at(i);

                        cdf[i] = counter;
                    }
                }

                if (min_cdf != -1)
                {
                    min_cdf = cdf.at(min_cdf);
                }

                if (max_cdf != -1)
                {
                    max_cdf = cdf.at(max_cdf);
                }

                boost::asynchronous::create_callback_continuation(
                    [result = std::move(result)](auto cont_res)
                    {
                        try
                        {
                            result.set_value(std::move(std::get<0>(cont_res).get()));
                        }
                        catch (...)
                        {
                            result.set_exception(std::current_exception());
                        }
                    },
                    equalize_image(std::move(image), cvpg::histogram<std::size_t> (std::move(cdf)), min_cdf, max_cdf)
                );
            },
            cvpg::imageproc::algorithms::tiling(std::move(tf))
        );
    }

private:
    cvpg::image_gray_8bit m_image;
};

}

namespace cvpg::imageproc::algorithms {

boost::asynchronous::detail::callback_continuation<image_gray_8bit> histogram_equalization(image_gray_8bit image)
{
    return boost::asynchronous::top_level_callback_continuation<image_gray_8bit>(
               histogram_equalization_task(std::move(image))
           );
}

} // namespace cvpg::imageproc::algoritms
