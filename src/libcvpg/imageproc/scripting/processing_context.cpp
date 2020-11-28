#include <libcvpg/imageproc/scripting/processing_context.hpp>

namespace cvpg { namespace imageproc { namespace scripting {

processing_context::processing_context(std::size_t id)
    : m_id(id)
{}

std::size_t processing_context::id() const
{
    return m_id;
}

void processing_context::store(std::uint32_t image_id, cvpg::image_gray_8bit image, std::chrono::microseconds duration)
{
    m_items[image_id] = item(item::types::grayscale_8_bit_image, std::move(image));
    m_durations[image_id] = std::move(duration);
    m_last_stored = image_id;
}

void processing_context::store(std::uint32_t image_id, cvpg::image_rgb_8bit image, std::chrono::microseconds duration)
{
    m_items[image_id] = item(item::types::rgb_8_bit_image, std::move(image));
    m_durations[image_id] = std::move(duration);
    m_last_stored = image_id;
}

item processing_context::load(std::uint32_t image_id) const
{
    auto it = m_items.find(image_id);

    if (it != m_items.end())
    {
        return it->second;
    }

    return item();
}

item processing_context::load() const
{
    auto it = m_items.find(m_last_stored);

    if (it != m_items.end())
    {
        return it->second;
    }

    return item();
}

void processing_context::add_parameter(std::string key, std::any value)
{
    m_params.insert({ std::move(key), std::move(value) });
}

void processing_context::set_parameters(parameters_type params)
{
    m_params = std::move(params);
}

processing_context::parameters_type processing_context::parameters() const
{
    return m_params;
}

}}} // namespace cvpg::imageproc::scripting
