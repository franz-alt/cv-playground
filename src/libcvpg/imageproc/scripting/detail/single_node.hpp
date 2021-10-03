// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_SINGLE_NODE_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_SINGLE_NODE_HPP

#include <libcvpg/imageproc/scripting/detail/node.hpp>

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>

#include <libcvpg/imageproc/scripting/detail/handler.hpp>
#include <libcvpg/imageproc/scripting/detail/single_node.hpp>

namespace cvpg::imageproc::scripting::detail {

class single_node : public node
{
public:
    single_node(std::uint32_t item_id, handler h);

    virtual ~single_node() override = default;

    // virtual void operator()(std::shared_ptr<scope> s) override;

    virtual std::string to_string() const override;

    std::uint32_t get_item_id() const;

private:
    std::uint32_t m_item_id;

    handler m_handler;
};

std::ostream & operator<<(std::ostream & out, single_node const & n);

} // namespace cvpg::imageproc::scripting::detail

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_SINGLE_NODE_HPP
