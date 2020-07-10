#include <libcvpg/imageproc/algorithms/convert_to_rgb.hpp>

#include <libcvpg/imageproc/algorithms/tiling.hpp>

namespace {

struct convert_to_rgb_8bit_task : public boost::asynchronous::continuation_task<cvpg::image_rgb_8bit>
{
    convert_to_rgb_8bit_task(cvpg::image_gray_8bit image)
        : boost::asynchronous::continuation_task<cvpg::image_rgb_8bit>("convert_to_rgb_8bit")
        , m_image(std::move(image))
    {}

    void operator()()
    {
        this->this_task_result().set_value(cvpg::image_rgb_8bit(m_image.width(),
                                                                m_image.height(),
                                                                m_image.padding(),
                                                                cvpg::image_rgb_8bit::channel_array_type { m_image.data(0), m_image.data(0), m_image.data(0) } ));
    }

private:
    cvpg::image_gray_8bit m_image;
};

}

namespace cvpg { namespace imageproc { namespace algorithms {

boost::asynchronous::detail::callback_continuation<image_rgb_8bit> convert_to_rgb(image_gray_8bit image)
{
    return boost::asynchronous::top_level_callback_continuation<image_rgb_8bit>(
               convert_to_rgb_8bit_task(std::move(image))
           );
}

}}} // namespace cvpg::imageproc::algoritms
