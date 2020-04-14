#include <libcvpg/imageproc/algorithms/border_mode.hpp>

#include <libcvpg/core/exception.hpp>

namespace cvpg { namespace imageproc { namespace algorithms {

std::ostream & operator<<(std::ostream & out, border_mode const & mode)
{
    switch (mode)
    {
        default:
        case border_mode::ignore:
            out << "ignore";
            break;

        case border_mode::constant:
            out << "constant";
            break;

        case border_mode::mirror:
            out << "mirror";
            break;
    }

    return out;
}

border_mode to_border_mode(std::string mode_str)
{
    if (mode_str == "ignore")
    {
        return border_mode::ignore;
    }
    else if (mode_str == "constant")
    {
        return border_mode::constant;
    }
    else if (mode_str == "mirror")
    {
        return border_mode::mirror;
    }

    throw cvpg::invalid_parameter_exception("invalid border mode");
}

}}} // namespace cvpg::imageproc::algoritms
