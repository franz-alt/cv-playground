#include <libcvpg/imageproc/algorithms/paint_meta.hpp>

#include <initializer_list>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <boost/algorithm/string.hpp>

#include <libcvpg/core/meta_data.hpp>
#include <libcvpg/core/multi_array.hpp>

namespace {

template<class input_image>
struct paint_meta_task : public boost::asynchronous::continuation_task<input_image>
{
    paint_meta_task(input_image image,
                    std::string key,
                    std::string mode,
                    std::string classes_key,
                    std::string classes_list,
                    std::string scores_key,
                    std::int32_t min_score,
                    std::initializer_list<std::uint8_t> colors)
        : boost::asynchronous::continuation_task<input_image>("paint_meta_task")
        , m_image(std::move(image))
        , m_key(std::move(key))
        , m_mode(std::move(mode))
        , m_classes_key(std::move(classes_key))
        , m_classes_list(std::move(classes_list))
        , m_scores_key(std::move(scores_key))
        , m_min_score(min_score)
        , m_colors(std::move(colors))
    {}

    void operator()()
    {
        // divide 'm_classes_list', a string with a comma separated list of classes, into a vector of classes
        std::vector<std::string> classes_keys;
        boost::split(classes_keys, m_classes_list, boost::is_any_of(","));

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

            // try to find 'paint_key' inside input image
            if (m_image.has_metadata())
            {
                auto const metadata = m_image.get_metadata();

                if (!!metadata)
                {
                    auto it_key_data = metadata->find(std::string(m_key).append(".data"));
                    auto it_key_dims = metadata->find(std::string(m_key).append(".dims"));
                    auto it_key_type = metadata->find(std::string(m_key).append(".type"));

                    auto it_classes_data = metadata->find(std::string(m_classes_key).append(".data"));
                    auto it_classes_dims = metadata->find(std::string(m_classes_key).append(".dims"));
                    auto it_classes_type = metadata->find(std::string(m_classes_key).append(".type"));

                    auto it_labels = metadata->find(std::string("labels"));

                    std::vector<std::size_t> class_ids;
                    class_ids.reserve(classes_keys.size());

                    if (it_labels != metadata->cend())
                    {
                        // TODO use a typdef for the unordered_map
                        auto label_map = std::any_cast<std::unordered_map<std::size_t, std::string> >(it_labels->second);

                        // determine the IDs of the label keys
                        for (auto const & class_key : classes_keys)
                        {
                            auto it = std::find_if(label_map.cbegin(),
                                                   label_map.cend(),
                                                   [&class_key](const auto & entry)
                                                   {
                                                       const auto & [id, key] = entry;
                                                       return class_key == key;
                                                   });

                            if (it != label_map.cend())
                            {
                                class_ids.push_back(it->first);
                            }
                        }
                    }

                    auto it_scores_data = metadata->find(std::string(m_scores_key).append(".data"));
                    auto it_scores_dims = metadata->find(std::string(m_classes_key).append(".dims"));
                    auto it_scores_type = metadata->find(std::string(m_classes_key).append(".type"));

                    if (it_key_data != metadata->cend() && it_key_dims != metadata->cend() && it_key_type != metadata->cend() &&
                        it_classes_data != metadata->cend() && it_classes_dims != metadata->cend() && it_classes_type != metadata->cend() &&
                        it_scores_data != metadata->cend() && it_scores_dims != metadata->cend() && it_scores_type != metadata->cend())
                    {
                        auto key_data = std::any_cast<cvpg::multi_array<float> >(it_key_data->second);
                        auto key_dims = std::any_cast<std::vector<int> >(it_key_dims->second);
                        auto key_type = std::any_cast<std::string>(it_key_type->second);

                        auto classes_data = std::any_cast<cvpg::multi_array<float> >(it_classes_data->second);
                        auto classes_dims = std::any_cast<std::vector<int> >(it_classes_dims->second);
                        auto classes_type = std::any_cast<std::string>(it_classes_type->second);

                        auto scores_data = std::any_cast<cvpg::multi_array<float> >(it_scores_data->second);
                        auto scores_dims = std::any_cast<std::vector<int> >(it_scores_dims->second);
                        auto scores_type = std::any_cast<std::string>(it_scores_type->second);

                        if (key_dims.size() == 3)
                        {
                            // TODO check for different dimensions!!!
                            // TODO check for equal amount of entries at keys, classes and scores!!!
                            auto [it, end] = key_data[0];
                            auto [it_classes, end_classes] = classes_data[0];
                            auto [it_scores, end_scores] = scores_data[0];

                            while (it != end)
                            {
                                // check if current object class is part of the desired classes
                                auto label_it = std::find_if(class_ids.cbegin(),
                                                             class_ids.cend(),
                                                             [classId = *it_classes++](auto const & id)
                                                             {
                                                                 return classId == id;
                                                             });

                                if (label_it == class_ids.cend())
                                {
                                    it += 4;
                                    ++it_scores;
                                    continue;
                                }

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

                                if (x2 > x1 && y2 > y1 && score >= (m_min_score / 100.0))
                                {
                                    std::size_t i = 0;

                                    if (m_mode == "rectangle")
                                    {
                                        for (auto const & color : m_colors)
                                        {
                                            memset((void*)(new_image.data(i).get() + y1 * new_image.width() + x1), color, x2 - x1);
                                            memset((void*)(new_image.data(i).get() + y2 * new_image.width() + x1), color, x2 - x1);

                                            for (std::size_t y = y1 + 1; y < y2; ++y)
                                            {
                                                memset((void*)(new_image.data(i).get() + y * new_image.width() + x1), color, 1);
                                                memset((void*)(new_image.data(i).get() + y * new_image.width() + x2 - 1), color, 1);
                                            }

                                            ++i;
                                        }
                                    }
                                    else
                                    {
                                        // TODO error handling
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
    std::string m_mode;

    std::string m_classes_key;
    std::string m_classes_list;

    std::string m_scores_key;
    std::int32_t m_min_score;

    std::initializer_list<std::uint8_t> m_colors;
};

}

namespace cvpg::imageproc::algorithms {

boost::asynchronous::detail::callback_continuation<image_gray_8bit> paint_meta(image_gray_8bit image,
                                                                               std::string key,
                                                                               std::string mode,
                                                                               std::string classes_key,
                                                                               std::string classes_list,
                                                                               std::string scores_key,
                                                                               std::int32_t min_score,
                                                                               std::uint8_t gray)
{
    return boost::asynchronous::top_level_callback_continuation<image_gray_8bit>(
               paint_meta_task(std::move(image), std::move(key), std::move(mode), std::move(classes_key), std::move(classes_list), std::move(scores_key), min_score, { gray })
           );
}

boost::asynchronous::detail::callback_continuation<image_rgb_8bit> paint_meta(image_rgb_8bit image,
                                                                              std::string key,
                                                                              std::string mode,
                                                                              std::string classes_key,
                                                                              std::string classes_list,
                                                                              std::string scores_key,
                                                                              std::int32_t min_score,
                                                                              std::uint8_t red,
                                                                              std::uint8_t green,
                                                                              std::uint8_t blue)
{
    return boost::asynchronous::top_level_callback_continuation<image_rgb_8bit>(
               paint_meta_task(std::move(image), std::move(key), std::move(mode), std::move(classes_key), std::move(classes_list), std::move(scores_key), min_score, { red, green, blue })
           );
}

} // namespace cvpg::imageproc::algorithms
