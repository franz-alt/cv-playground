#include <libcvpg/videoproc/packet.hpp>

#include <algorithm>

namespace cvpg::videoproc {

template<typename Frame> packet<Frame>::packet(std::size_t number)
    : m_number(number)
    , m_frames()
{}

template<typename Frame> std::size_t packet<Frame>::number() const
{
    return m_number;
}

template<typename Frame> void packet<Frame>::add_frame(Frame && frame)
{
    m_frames.push_back(std::forward<Frame>(frame));
}

template<typename Frame> std::vector<Frame> const & packet<Frame>::frames() const
{
    return m_frames;
}

template<typename Frame> std::vector<Frame> packet<Frame>::move_frames()
{
    return std::move(m_frames);
}

template<typename Frame> bool packet<Frame>::flush() const
{
    return std::find_if(m_frames.begin(),
                        m_frames.end(),
                        [](auto const & frame)
                        {
                            return frame.flush();
                        }) != m_frames.end();
}

template<typename Frame> std::ostream & operator<<(std::ostream & out, packet<Frame> const & packet)
{
    out << "number=" << packet.number() << ",frames=" << packet.frames().size();

    return out;
}

// manual instantation of packet<> for some types
template class packet<frame<image_gray_8bit> >;
template class packet<frame<image_rgb_8bit> >;

} // namespace cvpg::videoproc
