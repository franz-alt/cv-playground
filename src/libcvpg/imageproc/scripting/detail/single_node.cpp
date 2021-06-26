// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/imageproc/scripting/detail/single_node.hpp>

namespace cvpg { namespace imageproc { namespace scripting { namespace detail {

single_node::single_node(std::uint32_t item_id, handler h)
    : node()
    , m_item_id(item_id)
    , m_handler(std::move(h))
{}

// void single_node::operator()(std::shared_ptr<scope> s)
// {
//     // nothing to do here
// }

std::string single_node::to_string() const
{
    return std::to_string(m_item_id);
}

std::uint32_t single_node::get_item_id() const
{
    return m_item_id;
}

std::ostream & operator<<(std::ostream & out, single_node const & n)
{
    out << n.to_string();

    return out;
}

}}}} // namespace cvpg::imageproc::scripting::detail
