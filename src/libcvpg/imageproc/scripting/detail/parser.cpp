// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/imageproc/scripting/detail/parser.hpp>

#include <libcvpg/imageproc/scripting/algorithms/base.hpp>

namespace cvpg::imageproc::scripting::detail {

void parser::init(algorithm_set algorithms)
{
    m_algorithms = std::move(algorithms);

    for (auto const & algorithm : m_algorithms.all())
    {
        if (!!algorithm)
        {
            algorithm->on_parse(shared_from_this());
        }
    }
}

void parser::operator()(std::string expression)
{
    m_chai.eval(std::move(expression));
}

std::uint32_t parser::register_item(item i)
{
    auto new_id = ++m_item_id_counter;

    m_items.insert({ new_id, std::move(i) });

    return new_id;
}

void parser::register_link(std::uint32_t src_id, std::uint32_t dst_id)
{
    m_links[src_id].push_back(dst_id);
}

parser::item parser::find_item(std::uint32_t id) const
{
    auto it = m_items.find(id);

    if (it != m_items.end())
    {
        return it->second;
    }
    else
    {
        return item();
    }
}

std::unordered_map<std::uint32_t, parser::item> const & parser::items() const
{
    return m_items;
}

std::unordered_map<std::uint32_t, std::vector<std::uint32_t> > const & parser::links() const
{
    return m_links;
}

algorithm_set const & parser::algorithms() const
{
    return m_algorithms;
}

std::ostream & operator<<(std::ostream & out, parser::items_type const & items)
{
    for (auto const & item : items)
    {
        out << "id=" << item.first << ",name='" << item.second.name << "'" << ",arguments=" << item.second.arguments.size();
    }

    return out;
}

} // namespace cvpg::imageproc::scripting::detail
