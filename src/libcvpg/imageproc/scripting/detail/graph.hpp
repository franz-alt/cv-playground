// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_GRAPH_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_GRAPH_HPP

#include <cstdint>
#include <map>
#include <vector>

#include <libcvpg/imageproc/scripting/detail/sequence_node.hpp>

namespace cvpg { namespace imageproc { namespace scripting { namespace detail {

class graph
{
public:
    void add_link(std::uint32_t src_id, std::uint32_t dst_id);

    sequence_node to_sequence() const;

private:
    void process_node(std::uint32_t id, sequence_node & sequence) const;

    std::vector<std::uint32_t> get_predecessors(std::uint32_t id) const;

    bool has_successor(std::uint32_t node_id, std::uint32_t successor_id) const;

    std::map<std::uint32_t /*id*/, std::vector<std::uint32_t> /*neighbors*/> m_predecessors;
    std::map<std::uint32_t /*id*/, std::vector<std::uint32_t> /*neighbors*/> m_successors;
};

}}}} // namespace cvpg::imageproc::scripting::detail

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_GRAPH_HPP
