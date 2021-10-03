// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/videoproc/packet.hpp>

#include <algorithm>

namespace cvpg::videoproc {

template<typename Frame> packet<Frame>::packet(std::size_t number, bool failed)
    : m_number(number)
    , m_frames()
    , m_failed(failed)
{}

template<typename Frame> std::size_t packet<Frame>::number() const
{
    return m_number;
}

template<typename Frame> void packet<Frame>::add_frame(frame_type && frame)
{
    m_frames.push_back(std::forward<Frame>(frame));
}

template<typename Frame> std::vector<typename packet<Frame>::frame_type> const & packet<Frame>::frames() const
{
    return m_frames;
}

template<typename Frame> std::vector<typename packet<Frame>::frame_type> packet<Frame>::move_frames()
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

template<typename Frame> bool packet<Frame>::failed() const
{
    return m_failed;
}

// manual instantiation of packet<> for some types
template class packet<frame<image_gray_8bit> >;
template class packet<frame<image_rgb_8bit> >;

template<typename Frame> std::ostream & operator<<(std::ostream & out, packet<Frame> const & packet)
{
    out << "number=" << packet.number() << ",frames=" << packet.frames().size() << ",flush=" << packet.flush() << ",failed=" << packet.failed();

    return out;
}

// manual instantiation of operator<< for some types
template std::ostream & operator<< <frame<image_gray_8bit> >(std::ostream &, packet<frame<image_gray_8bit> > const &);
template std::ostream & operator<< <frame<image_rgb_8bit> >(std::ostream &, packet<frame<image_rgb_8bit> > const &);

} // namespace cvpg::videoproc
