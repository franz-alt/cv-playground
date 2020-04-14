#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_BORDER_MODE_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_BORDER_MODE_HPP

#include <ostream>
#include <string>

namespace cvpg { namespace imageproc { namespace algorithms {

enum class border_mode
{
    ignore,     // ignore borders
    constant,   // constant values at border
    mirror      // use the pixel values from opposite side
};

std::ostream & operator<<(std::ostream & out, border_mode const & mode);

border_mode to_border_mode(std::string mode_str);

}}} // namespace cvpg::imageproc::algoritms

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_BORDER_MODE_HPP
