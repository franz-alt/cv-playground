#include <libcvpg/videoproc/frame.hpp>

namespace cvpg::videoproc {

template<typename Image> frame<Image>::frame(std::size_t number)
    : m_number(number)
    , m_image()
    , m_flush(true)
{}

template<typename Image> frame<Image>::frame(std::size_t number, Image && image)
    : m_number(number)
    , m_image(std::forward<Image>(image))
    , m_flush(false)
{}

template<typename Image> std::size_t frame<Image>::number() const
{
    return m_number;
}

template<typename Image> Image frame<Image>::image() const
{
    return m_image;
}

template<typename Image> bool frame<Image>::flush() const
{
    return m_flush;
}

template<typename Image> std::ostream & operator<<(std::ostream & out, frame<Image> const & frame)
{
    out << "number=" << frame.number() << ",image=" << frame.image() << ",flush=" << frame.flush();

    return out;
}

// manual instantation of frame<> for some types
template class frame<image_gray_8bit>;
template class frame<image_rgb_8bit>;

} // namespace cvpg::videoproc
