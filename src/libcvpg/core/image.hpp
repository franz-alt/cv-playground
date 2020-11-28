#ifndef LIBCVPG_CORE_IMAGE_HPP
#define LIBCVPG_CORE_IMAGE_HPP

#include <any>
#include <array>
#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <tuple>

namespace cvpg {

template<class pixel = std::uint8_t, std::uint8_t channels = 1>
class image
{
public:
    using pixel_type = pixel;

    using channel_array_type = std::array<std::shared_ptr<pixel_type>, channels>;

    image(std::uint32_t width = 0, std::uint32_t height = 0, std::uint32_t padding = 0);

    image(std::uint32_t width, std::uint32_t height, std::uint32_t padding, channel_array_type data);

    image(image const &) = default;
    image(image &&) = default;

    image & operator=(image const &) = default;
    image & operator=(image &&) = default;

    std::uint32_t width() const;

    std::uint32_t height() const;

    std::uint32_t padding() const;

    std::shared_ptr<pixel_type> data(std::uint8_t channel) const;

private:
    std::uint32_t m_width = 0;
    std::uint32_t m_height = 0;

    std::uint32_t m_padding = 0;

    channel_array_type m_data;
};

using image_gray_8bit = image<std::uint8_t, 1>;
using image_rgb_8bit = image<std::uint8_t, 3>;

image_gray_8bit read_gray_8bit_png(std::string const & filename);
image_rgb_8bit read_rgb_8bit_png(std::string const & filename);

std::tuple<std::uint8_t, std::any> read_png(std::string const & filename);

void write_png(image_gray_8bit const & img, std::string const & filename);
void write_png(image_rgb_8bit const & img, std::string const & filename);

// suppress automatic instantiation of image<> for some types
extern template class image<std::uint8_t, 1>;
extern template class image<std::uint8_t, 3>;

std::ostream & operator<<(std::ostream & out, image_gray_8bit const & i);
std::ostream & operator<<(std::ostream & out, image_rgb_8bit const & i);

} // namespace cvpg

#endif // LIBCVPG_CORE_IMAGE_HPP
