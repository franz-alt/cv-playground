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
    frame(std::size_t number, image_type && image);

    // get the number of the frame
    std::size_t number() const;

    // get the image stored at the frame
    image_type image() const;

    // check if the frame is a flush frame
    bool flush() const;

private:
    std::size_t m_number = 0;

    image_type m_image;

    bool m_flush = false;
};

// suppress automatic instantiation of frame<> for some types
extern template class frame<image_gray_8bit>;
extern template class frame<image_rgb_8bit>;

template<typename Image>
std::ostream & operator<<(std::ostream & out, frame<Image> const & frame);

// suppress automatic instantiation of operator<< for some types
extern template std::ostream & operator<< <image_gray_8bit>(std::ostream &, frame<image_gray_8bit> const &);
extern template std::ostream & operator<< <image_rgb_8bit>(std::ostream &, frame<image_rgb_8bit> const &);

template<typename Image>
bool operator<(frame<Image> const & a, frame<Image> const & b);

// suppress automatic instantiation of operator< for some types
extern template bool operator< <image_gray_8bit>(frame<image_gray_8bit> const &, frame<image_gray_8bit> const &);
extern template bool operator< <image_rgb_8bit>(frame<image_rgb_8bit> const &, frame<image_rgb_8bit> const &);

template<typename Image>
bool operator>(frame<Image> const & a, frame<Image> const & b);

// suppress automatic instantiation of operator> for some types
extern template bool operator> <image_gray_8bit>(frame<image_gray_8bit> const &, frame<image_gray_8bit> const &);
extern template bool operator> <image_rgb_8bit>(frame<image_rgb_8bit> const &, frame<image_rgb_8bit> const &);

} // namespace cvpg::videoproc

#endif // LIBCVPG_VIDEOPROC_FRAME_HPP
