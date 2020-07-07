#ifndef LIBCVPG_VIDEOPROC_FRAME_HPP
#define LIBCVPG_VIDEOPROC_FRAME_HPP

#include <libcvpg/core/image.hpp>

#include <ostream>

namespace cvpg::videoproc {

//
// A frame represents a single image inside a video stream. Frames consists of the image data and
// a number of the frame inside the video stream.
//
// If a video stream is finished a so called 'flush' frame indicates the end of video stream.
//
template<typename Image>
class frame
{
public:
    using image_type = Image;

    // create an invalid frame with an empty image and frame number 0
    frame() = default;

    // create a flush frame with a specified number
    frame(std::size_t number);

    // create a normal frame with a specified number and an image
    frame(std::size_t number, Image && image);

    // get the number of the frame
    std::size_t number() const;

    // get the image stored at the frame
    Image image() const;

    // check if the frame is a flush frame
    bool flush() const;

private:
    std::size_t m_number = 0;

    Image m_image;

    bool m_flush = false;
};

template<typename Image>
std::ostream & operator<<(std::ostream & out, frame<Image> const & frame);

extern template class frame<image_gray_8bit>;
extern template class frame<image_rgb_8bit>;

} // namespace cvpg::videoproc

#endif // LIBCVPG_VIDEOPROC_FRAME_HPP
