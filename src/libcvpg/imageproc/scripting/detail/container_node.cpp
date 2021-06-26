// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/imageproc/scripting/detail/container_node.hpp>

namespace cvpg { namespace imageproc { namespace scripting { namespace detail {

std::ostream & operator<<(std::ostream & out, container_node const & n)
{
    out << n;

    return out;
}

}}}} // namespace cvpg::imageproc::scripting::detail
