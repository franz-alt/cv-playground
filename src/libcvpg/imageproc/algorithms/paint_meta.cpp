#include <libcvpg/imageproc/algorithms/paint_meta.hpp>

#include <type_traits>

#include <libcvpg/core/meta_data.hpp>
#include <libcvpg/core/multi_array.hpp>

namespace {

template<class input_image>
struct paint_meta_task : public boost::asynchronous::continuation_task<input_image>
{
    paint_meta_task(input_image image, std::string key, std::string scores, std::string mode)
        : boost::asynchronous::continuation_task<input_image>("paint_meta_task")
        , m_image(std::move(image))
        , m_key(std::move(key))
        , m_scores(std::move(scores))
        , m_mode(std::move(mode))
    {}

    void operator()()
    {
        try
        {
            // create a new black image with the same dimensions as the input image
            input_image new_image(m_image.width(), m_image.height());

            const std::size_t bytes_per_channel = new_image.width() * new_image.height();

            if constexpr (std::is_same<input_image, cvpg::image_gray_8bit>::value)
            {
                memset((void*)new_image.data(0).get(), 0, bytes_per_channel);
            }
            else if constexpr (std::is_same<input_image, cvpg::image_rgb_8bit>::value)
            {
                memset((void*)new_image.data(0).get(), 0, bytes_per_channel);
                memset((void*)new_image.data(1).get(), 0, bytes_per_channel);
                memset((void*)new_image.data(2).get(), 0, bytes_per_channel);
            }
            else
            {
                // TODO handle error
            }

            // try to find 'key' inside input image
            if (m_image.has_metadata())
            {
                auto const metadata = m_image.get_metadata();

                if (!!metadata)
                {
                    auto it_key_data = metadata->find(std::string(m_key).append(".data"));
                    auto it_key_dims = metadata->find(std::string(m_key).append(".dims"));
                    auto it_key_type = metadata->find(std::string(m_key).append(".type"));

                    auto it_scores_data = metadata->find(std::string(m_scores).append(".data"));
                    auto it_scores_dims = metadata->find(std::string(m_scores).append(".dims"));
                    auto it_scores_type = metadata->find(std::string(m_scores).append(".type"));

                    if (it_key_data != metadata->cend() && it_key_dims != metadata->cend() && it_key_type != metadata->cend() &&
                        it_scores_data != metadata->cend() && it_scores_dims != metadata->cend() && it_scores_type != metadata->cend())
                    {
                        auto key_data = std::any_cast<cvpg::multi_array<float> >(it_key_data->second);
                        auto key_dims = std::any_cast<std::vector<int> >(it_key_dims->second);
                        auto key_type = std::any_cast<std::string>(it_key_type->second);

                        auto scores_data = std::any_cast<cvpg::multi_array<float> >(it_scores_data->second);
                        auto scores_dims = std::any_cast<std::vector<int> >(it_scores_dims->second);
                        auto scores_type = std::any_cast<std::string>(it_scores_type->second);

                        if (key_dims.size() == 3)
                        {
                            // TODO check for different dimensions!!!
                            auto [it, end] = key_data[0];
                            auto [it_scores, end_scores] = scores_data[0];

                            while (it != end)
                            {
                                // boxes are interpreted as [y_min, x_min, y_max, x_max] ...
                                auto y1 = static_cast<std::size_t>(*it++ * new_image.height());
                                auto x1 = static_cast<std::size_t>(*it++ * new_image.width());
                                auto y2 = static_cast<std::size_t>(*it++ * new_image.height());
                                auto x2 = static_cast<std::size_t>(*it++ * new_image.width());

                                // ... and corrected (if nescessary)
                                x1 = std::min(x1, x2);
                                y1 = std::min(y1, y2);
                                x2 = std::max(x1, x2);
                                y2 = std::max(y1, y2);

                                const auto score = *it_scores++;

                                if (x2 > x1 && y2 > y1 && score > 0.1)
                                {
                                    memset((void*)(new_image.data(0).get() + y1 * new_image.width() + x1), 255, x2 - x1);
                                    memset((void*)(new_image.data(0).get() + y2 * new_image.width() + x1), 255, x2 - x1);

                                    for (std::size_t y = y1 + 1; y < y2; ++y)
                                    {
                                        memset((void*)(new_image.data(0).get() + y * new_image.width() + x1), 255, 1);
                                        memset((void*)(new_image.data(0).get() + y * new_image.width() + x2 - 1), 255, 1);
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        // TODO error handling
                    }
                }
                else
                {
                    // TODO error handling
                }
            }
            else
            {
                // don't do anything here
            }

            this->this_task_result().set_value(std::move(new_image));
        }
        catch (...)
        {
            this->this_task_result().set_exception(std::current_exception());
        }
    }

private:
    input_image m_image;

    std::string m_key;
    std::string m_scores;

    std::string m_mode;
};

}

namespace cvpg::imageproc::algorithms {

boost::asynchronous::detail::callback_continuation<image_gray_8bit> paint_meta(image_gray_8bit image, std::string key, std::string scores, std::string mode)
{
    return boost::asynchronous::top_level_callback_continuation<image_gray_8bit>(
               paint_meta_task(std::move(image), std::move(key), std::move(scores), std::move(mode))
           );
}

boost::asynchronous::detail::callback_continuation<image_rgb_8bit> paint_meta(image_rgb_8bit image, std::string key, std::string scores, std::string mode)
{
    return boost::asynchronous::top_level_callback_continuation<image_rgb_8bit>(
               paint_meta_task(std::move(image), std::move(key), std::move(scores), std::move(mode))
           );
}

} // namespace cvpg::imageproc::algorithms
