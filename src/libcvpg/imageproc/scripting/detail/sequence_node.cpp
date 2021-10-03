// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/imageproc/scripting/detail/sequence_node.hpp>

#include <sstream>

namespace cvpg::imageproc::scripting::detail {

// void sequence_node::operator()(std::shared_ptr<scope> s)
// {
//     // nothing to do here
// }

std::string sequence_node::to_string() const
{
    std::stringstream ss;

    ss << "Seq(";

    for (auto const & node : m_nodes)
    {
        ss << node->to_string() << " ";
    }

    ss << ")";

    return ss.str();
}

void sequence_node::push_front(std::shared_ptr<node> n)
{
    m_nodes.push_front(std::move(n));
}

void sequence_node::push_back(std::shared_ptr<node> n)
{
    m_nodes.push_back(std::move(n));
}

std::size_t sequence_node::size() const
{
    return m_nodes.size();
}

std::deque<std::shared_ptr<node> >::iterator sequence_node::begin()
{
    return m_nodes.begin();
}

std::deque<std::shared_ptr<node> >::iterator sequence_node::end()
{
    return m_nodes.end();
}

std::deque<std::shared_ptr<node> >::const_iterator sequence_node::cbegin() const
{
    return m_nodes.cbegin();
}

std::deque<std::shared_ptr<node> >::const_iterator sequence_node::cend() const
{
    return m_nodes.cend();
}

std::shared_ptr<container_node> sequence_node::find_container(std::uint32_t id) const
{
    for (auto const & node : m_nodes)
    {
        auto * container = dynamic_cast<container_node*>(node.get());

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

std::ostream & operator<<(std::ostream & out, sequence_node const & s)
{
    out << s.to_string();

    return out;
}

} // namespace cvpg::imageproc::scripting::detail
