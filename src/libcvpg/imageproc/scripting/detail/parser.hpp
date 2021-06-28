// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_PARSER_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_PARSER_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <libcvpg/imageproc/scripting/item.hpp>
#include <libcvpg/imageproc/scripting/algorithm_set.hpp>

#include <chaiscript/chaiscript.hpp>

namespace cvpg::imageproc::scripting::detail {

class parser : public std::enable_shared_from_this<parser>
{
public:
    struct item
    {
        std::string name;
        std::vector<scripting::item> arguments;
    };

    using items_type = std::unordered_map<std::uint32_t, item>;
    using links_type = std::unordered_map<std::uint32_t, std::vector<std::uint32_t> >;

    void init(algorithm_set algorithms);

    template<class ... Ts>
    void register_specification(std::string name, std::function<std::uint32_t(Ts...)> handler)
    {
        m_chai.add(chaiscript::fun(std::move(handler)), std::move(name));
    }

    void operator()(std::string expression);

    std::uint32_t register_item(item i);

    void register_link(std::uint32_t src_id, std::uint32_t dst_id);

    item find_item(std::uint32_t id) const;

    items_type const & items() const;
    links_type const & links() const;

    algorithm_set const & algorithms() const;

private:
    algorithm_set m_algorithms;

    items_type m_items;
    links_type m_links;

    std::uint32_t m_item_id_counter = 0;

    chaiscript::ChaiScript m_chai;
};

std::ostream & operator<<(std::ostream & out, parser::items_type const & items);

} // namespace cvpg::imageproc::scripting::detail

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_PARSER_HPP
