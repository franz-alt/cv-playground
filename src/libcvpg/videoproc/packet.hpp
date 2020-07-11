#ifndef LIBCVPG_VIDEOPROC_PACKET_HPP
#define LIBCVPG_VIDEOPROC_PACKET_HPP

#include <libcvpg/videoproc/frame.hpp>

#include <ostream>
#include <vector>

namespace cvpg::videoproc {

//
// A packet containing several frames of a video stream. If a packet is the last packet of a video
// stream it contains a flush frame and therefore is marked as a 'flush' packet.
//
template<typename Frame>
class packet
{
public:
    using frame_type = Frame;

    // create a packet with a specified number
    packet(std::size_t number = 0);

    // get the number of the frame
    std::size_t number() const;

    // move an existing frame to the packet
    void add_frame(Frame && frame);

    // create a new frame in-place the packet
    template<typename... Args>
    void add_frame(Args && ... args)
    {
        m_frames.emplace_back(std::forward<Args>(args)...);
    }
    
    // get a reference to the frames inside the packet
    std::vector<Frame> const & frames() const;

    // move out all frames inside the packet
    std::vector<Frame> move_frames();

    // check if the packet contains flush frames
    bool flush() const;

private:
    std::size_t m_number;

    std::vector<Frame> m_frames;
};

// suppress automatic instantiation of packet<> for some types
extern template class packet<frame<image_gray_8bit> >;
extern template class packet<frame<image_rgb_8bit> >;

template<typename Frame>
std::ostream & operator<<(std::ostream & out, packet<Frame> const & packet);

// suppress automatic instantiation of operator<< for some types
extern template std::ostream & operator<< <frame<image_gray_8bit> >(std::ostream &, packet<frame<image_gray_8bit> > const &);
extern template std::ostream & operator<< <frame<image_rgb_8bit> >(std::ostream &, packet<frame<image_rgb_8bit> > const &);

} // namespace cvpg::videoproc

#endif // LIBCVPG_VIDEOPROC_PACKET_HPP
