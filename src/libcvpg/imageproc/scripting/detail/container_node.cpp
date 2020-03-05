#include <libcvpg/imageproc/scripting/detail/container_node.hpp>

namespace cvpg { namespace imageproc { namespace scripting { namespace detail {

std::ostream & operator<<(std::ostream & out, container_node const & n)
{
    out << n;

    return out;
}

}}}} // namespace cvpg::imageproc::scripting::detail
