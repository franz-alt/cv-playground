#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_PROCESSING_CONTEXT_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_PROCESSING_CONTEXT_HPP

#include <any>
#include <chrono>
#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>

#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/scripting/item.hpp>

namespace cvpg { namespace imageproc { namespace scripting {

class processing_context
{
public:
    using parameters_type = std::map<std::string, std::any>;

    processing_context(std::size_t id);

    std::size_t id() const;

    // store an image
    void store(std::uint32_t image_id, cvpg::image_gray_8bit image, std::chrono::microseconds duration = std::chrono::microseconds());
    void store(std::uint32_t image_id, cvpg::image_rgb_8bit image, std::chrono::microseconds duration = std::chrono::microseconds());

    // load an item with a specific ID
    item load(std::uint32_t image_id) const;

    // load last stored item
    item load() const;

    // add parameters
    void add_parameter(std::string key, std::any value);

    // set parameters
    void set_parameters(parameters_type params);

    // get all parameters
    parameters_type parameters() const;

private:
    std::size_t m_id = 0;

    std::unordered_map<std::uint32_t, item> m_items;
    std::unordered_map<std::uint32_t, std::chrono::microseconds> m_durations;

    std::uint32_t m_last_stored = 0;

    parameters_type m_params;
};

}}} // namespace cvpg::imageproc::scripting

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_PROCESSING_CONTEXT_HPP
