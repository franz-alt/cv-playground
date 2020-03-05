#include <libcvpg/imageproc/algorithms/convert_to_gray.hpp>

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
            // TODO implement me
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
