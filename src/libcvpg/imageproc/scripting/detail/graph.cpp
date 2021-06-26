// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/imageproc/scripting/detail/graph.hpp>

#include <algorithm>
#include <memory>

#include <libcvpg/imageproc/scripting/detail/container_node.hpp>
#include <libcvpg/imageproc/scripting/detail/parallel_node.hpp>
#include <libcvpg/imageproc/scripting/detail/single_node.hpp>

namespace {

struct node
{
    std::uint32_t value = 0;

    std::weak_ptr<node> parent;

    std::vector<std::shared_ptr<node> > children;
};

}

namespace cvpg { namespace imageproc { namespace scripting { namespace detail {

void graph::add_link(std::uint32_t src_id, std::uint32_t dst_id)
{
    // update successors
    m_successors[src_id].push_back(dst_id);

    // check if destination node already exists
    if (m_successors.find(dst_id) == m_successors.end())
    {
        // create empty node
        m_successors.insert({ dst_id, std::vector<std::uint32_t>() });
    }

    // update predecessors
    m_predecessors[dst_id].push_back(src_id);

    // check if source node already exists
    if (m_predecessors.find(src_id) == m_predecessors.end())
    {
        // create empty node
        m_predecessors.insert({ src_id, std::vector<std::uint32_t>() });
    }
}

sequence_node graph::to_sequence() const
{
    // determine all nodes that have no successors
    std::vector<std::uint32_t> no_successors;
    no_successors.reserve(m_successors.size());

    for (auto const & node : m_successors)
    {
        if (node.second.empty())
        {
            no_successors.push_back(node.first);
        }
    }

    sequence_node sequence;

    for (auto const & id : no_successors)
    {
        process_node(id, sequence);
    }

    return sequence;
}

void graph::process_node(std::uint32_t id, sequence_node & sequence) const
{
    auto predecessors = get_predecessors(id);

    if (predecessors.empty())
    {
        sequence.push_front(std::make_shared<single_node>(id, handler()));

        return;
    }
    else if (predecessors.size() == 1)
    {
        sequence.push_front(std::make_shared<single_node>(id, handler()));

        process_node(predecessors.front(), sequence);

        return;
    }

    // collect all predecessors that are no successor of other predecessors
    std::vector<std::uint32_t> filtered_predecessors;
    filtered_predecessors.reserve(predecessors.size());

    for (auto const & p : predecessors)
    {
        // check if 'p' has no other 'predecessors' of 'id' as successor
        auto it = std::find_if(predecessors.begin(),
                               predecessors.end(),
                               [this, id = p](std::uint32_t predecessor_id)
                               {
                                   return has_successor(id, predecessor_id);
                               });

        // is 'p' successor of other predecessor!?
        bool is_soop = it != predecessors.end();

        if (!is_soop)
        {
            filtered_predecessors.push_back(p);
        }
    }

    if (filtered_predecessors.empty())
    {
        return;
    }
    else if (filtered_predecessors.size() == 1)
    {
        sequence.push_front(std::make_shared<single_node>(id, handler()));

        process_node(filtered_predecessors.front(), sequence);

        return;
    }

    auto parallel = std::make_shared<parallel_node>();

    for (auto const & p : filtered_predecessors)
    {
        auto container = sequence.find_container(p);

        if (!!container)
        {
            container->push_front(std::make_shared<single_node>(p, handler()));

            process_node(p, sequence);
        }
        else
        {
            auto sub_sequence = std::make_shared<sequence_node>();
            parallel->push_back(sub_sequence);

            process_node(p, *sub_sequence);
        }
    }

    sequence.push_front(std::make_shared<single_node>(id, handler()));
    sequence.push_front(parallel);
}

std::vector<std::uint32_t> graph::get_predecessors(std::uint32_t id) const
{
    auto it = m_predecessors.find(id);

    if (it != m_predecessors.end())
    {
        return it->second;
    }

    return std::vector<std::uint32_t>();
}

bool graph::has_successor(std::uint32_t node_id, std::uint32_t successor_id) const
{
    auto it = m_successors.find(node_id);

    if (it != m_successors.end())
    {
        auto successors = it->second;

        auto it = std::find(successors.begin(), successors.end(), successor_id);

        return it != successors.end();
    }

    return false;
}

}}}} // namespace cvpg::imageproc::scripting::detail
