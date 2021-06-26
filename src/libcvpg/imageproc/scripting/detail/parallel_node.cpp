// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/imageproc/scripting/detail/parallel_node.hpp>

#include <sstream>

namespace cvpg { namespace imageproc { namespace scripting { namespace detail {

// void parallel_node::operator()(std::shared_ptr<scope> s)
// {
//     // nothing to do here
// }

std::string parallel_node::to_string() const
{
    std::stringstream ss;

    ss << "Par(";

    for (auto const & node : m_nodes)
    {
        ss << node->to_string() << " ";
    }

    ss << ")";

    return ss.str();
}

void parallel_node::push_front(std::shared_ptr<node> n)
{
    m_nodes.push_front(std::move(n));
}

void parallel_node::push_back(std::shared_ptr<node> n)
{
    m_nodes.push_back(std::move(n));
}

std::size_t parallel_node::size() const
{
    return m_nodes.size();
}

std::deque<std::shared_ptr<node> >::iterator parallel_node::begin()
{
    return m_nodes.begin();
}

std::deque<std::shared_ptr<node> >::iterator parallel_node::end()
{
    return m_nodes.end();
}

std::deque<std::shared_ptr<node> >::const_iterator parallel_node::cbegin() const
{
    return m_nodes.cbegin();
}

std::deque<std::shared_ptr<node> >::const_iterator parallel_node::cend() const
{
    return m_nodes.cend();
}

std::shared_ptr<container_node> parallel_node::find_container(std::uint32_t id) const
{
    for (auto const & node : m_nodes)
    {
        auto * container = dynamic_cast<container_node *>(node.get());

        if (container != nullptr)
        {
            auto res = container->find_container(id);

            if (!!res)
            {
                return res;
            }
        }
    }

    return std::shared_ptr<container_node>();
}

std::ostream & operator<<(std::ostream & out, parallel_node const & p)
{
    out << p.to_string();

    return out;
}

}}}} // namespace cvpg::imageproc::scripting::detail
