// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/imageproc/algorithms/convert_to_gray.hpp>

#include <libcvpg/imageproc/algorithms/tiling.hpp>
#include <libcvpg/imageproc/algorithms/tiling/convert_to_gray.hpp>

namespace {

struct convert_to_gray_8bit_task : public boost::asynchronous::continuation_task<cvpg::image_gray_8bit>
{
    convert_to_gray_8bit_task(cvpg::image_rgb_8bit image, cvpg::imageproc::algorithms::rgb_conversion_mode mode)
        : boost::asynchronous::continuation_task<cvpg::image_gray_8bit>("convert_to_gray_8bit")
        , m_image(std::move(image))
        , m_mode(mode)
    {}

    void operator()()
    {
        if (m_mode == cvpg::imageproc::algorithms::rgb_conversion_mode::use_red)
        {
            this->this_task_result().set_value(cvpg::image_gray_8bit(m_image.width(),
                                                                     m_image.height(),
                                                                     m_image.padding(),
                                                                     cvpg::image_gray_8bit::channel_array_type { m_image.data(0) } ));
        }
        else if (m_mode == cvpg::imageproc::algorithms::rgb_conversion_mode::use_green)
        {
            this->this_task_result().set_value(cvpg::image_gray_8bit(m_image.width(),
                                                                     m_image.height(),
                                                                     m_image.padding(),
                                                                     cvpg::image_gray_8bit::channel_array_type { m_image.data(1) } ));
        }
        else if (m_mode == cvpg::imageproc::algorithms::rgb_conversion_mode::use_blue)
        {
            this->this_task_result().set_value(cvpg::image_gray_8bit(m_image.width(),
                                                                     m_image.height(),
                                                                     m_image.padding(),
                                                                     cvpg::image_gray_8bit::channel_array_type { m_image.data(2) } ));
        }
        else if (m_mode == cvpg::imageproc::algorithms::rgb_conversion_mode::calc_average)
        {
            const auto width = m_image.width();
            const auto height = m_image.height();

            auto tf = cvpg::imageproc::algorithms::tiling_functors::image<cvpg::image_rgb_8bit, cvpg::image_gray_8bit>({{ std::move(m_image) }});
            tf.parameters.image_width = width;
            tf.parameters.image_height = height;
            tf.parameters.cutoff_x = 512;
            tf.parameters.cutoff_y = 512;

            tf.tile_algorithm_task = [](std::shared_ptr<cvpg::image_rgb_8bit> src1, std::shared_ptr<cvpg::image_rgb_8bit> /*src2*/, std::shared_ptr<cvpg::image_gray_8bit> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
            {
                cvpg::imageproc::algorithms::convert_to_gray_8bit(src1->data(0).get(), src1->data(1).get(), src1->data(2).get(), dst->data(0).get(), from_x, to_x, from_y, to_y, std::move(parameters));
            };

            boost::asynchronous::create_callback_continuation(
                [task_result = this->this_task_result()](auto result)
                {
                    auto image = std::move(std::get<0>(result).get());

                    task_result.set_value(cvpg::image_gray_8bit(image.width(),
                                                                image.height(),
                                                                image.padding(),
                                                                cvpg::image_gray_8bit::channel_array_type { image.data(0) } ));
                },
                cvpg::imageproc::algorithms::tiling(std::move(tf))
            );
        }
        else
        {
            // TODO error handling
        }
    }

private:
    cvpg::image_rgb_8bit m_image;

    cvpg::imageproc::algorithms::rgb_conversion_mode m_mode;
};

}

namespace cvpg { namespace imageproc { namespace algorithms {

std::ostream & operator<<(std::ostream & out, rgb_conversion_mode const & mode)
{
    switch (mode)
    {
        case rgb_conversion_mode::use_red:
            out << "use red";
            break;

        case rgb_conversion_mode::use_green:
            out << "use green";
            break;

        case rgb_conversion_mode::use_blue:
            out << "use blue";
            break;

        case rgb_conversion_mode::calc_average:
            out << "calculate average";
            break;
    }

    return out;
}

boost::asynchronous::detail::callback_continuation<image_gray_8bit> convert_to_gray(image_rgb_8bit image, rgb_conversion_mode mode)
{
    return boost::asynchronous::top_level_callback_continuation<image_gray_8bit>(
               convert_to_gray_8bit_task(std::move(image), mode)
           );
}

}}} // namespace cvpg::imageproc::algoritms
