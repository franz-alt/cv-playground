// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/imageproc/scripting/detail/compiler.hpp>

#include <libcvpg/imageproc/scripting/detail/graph.hpp>
#include <libcvpg/imageproc/scripting/algorithms/base.hpp>

namespace cvpg { namespace imageproc { namespace scripting { namespace detail {

compiler::compiler(algorithm_set algorithms)
    : m_parser(new parser())
{
    m_parser->init(std::move(algorithms));
}

compiler::result compiler::operator()()
{
    if (m_parser->items().empty())
    {
        return result();
    }

    // build a compile graph
    graph g;

    for (auto const & link : m_parser->links())
    {
        for (auto const & successor_id : link.second)
        {
            g.add_link(link.first, successor_id);
        }
    }

    auto seq = g.to_sequence();

    // register the handlers
    for (auto const & item : m_parser->items())
    {
        auto spec = m_parser->algorithms().find(item.second.name);

        if (!!spec)
        {
            spec->on_compile(item.first, shared_from_this());
        }
    }

    result res;
    res.flow = std::make_shared<sequence_node>(std::move(seq));
    res.handlers = m_handlers;

    return res;
}

void compiler::register_handler(std::uint32_t item_id, std::string name, handler h)
{
    auto it = m_handlers.find(item_id);

    if (it == m_handlers.end())
    {
        m_handlers.insert({ item_id, std::move(h) });
    }
}

handler compiler::get_handler(std::uint32_t item_id) const
{
    handler h;

    auto it = m_handlers.find(item_id);

    if (it != m_handlers.end())
    {
        h = it->second;
    }

    return h;
}

parser::item compiler::get_item(std::uint32_t item_id) const
{
    auto const & items = m_parser->items();

    auto it = items.find(item_id);

    if (it != items.end())
    {
        return it->second;
    }

    return parser::item();
}

std::shared_ptr<detail::parser> compiler::get_parser() const
{
    return m_parser;
}

}}}} // namespace cvpg::imageproc::scripting::detail
