// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_COMPILER_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_COMPILER_HPP

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <libcvpg/imageproc/scripting/algorithm_set.hpp>
#include <libcvpg/imageproc/scripting/detail/handler.hpp>
#include <libcvpg/imageproc/scripting/detail/parser.hpp>

namespace cvpg::imageproc::scripting::detail {

class sequence_node;

class compiler : public std::enable_shared_from_this<compiler>
{
public:
    struct result
    {
        std::shared_ptr<sequence_node> flow;
        std::unordered_map<std::uint32_t, handler> handlers;
    };

    compiler(algorithm_set algorithms);

    result operator()();

    void register_handler(std::uint32_t item_id, std::string name, handler h);

    handler get_handler(std::uint32_t item_id) const;

    parser::item get_item(std::uint32_t item_id) const;

    std::shared_ptr<detail::parser> get_parser() const;

private:
    std::shared_ptr<detail::parser> m_parser;

    std::unordered_map<std::uint32_t, parser::item> m_items;

    std::unordered_map<std::uint32_t, handler> m_handlers;

    std::unordered_set<std::uint32_t> m_compiled;
};

} // namespace cvpg::imageproc::scripting::detail

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_COMPILER_HPP
