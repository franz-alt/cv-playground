// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_PARALLEL_NODE_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_PARALLEL_NODE_HPP

#include <libcvpg/imageproc/scripting/detail/container_node.hpp>

#include <cstdint>
#include <deque>
#include <memory>
#include <ostream>
#include <string>

namespace cvpg::imageproc::scripting::detail {

class parallel_node : public container_node
{
public:
    virtual ~parallel_node() override = default;

    // virtual void operator()(std::shared_ptr<scope> s) override;

    virtual std::string to_string() const override;

    virtual void push_front(std::shared_ptr<node> n) override;
    virtual void push_back(std::shared_ptr<node> n) override;

    virtual std::size_t size() const override;

    virtual std::deque<std::shared_ptr<node> >::iterator begin() override;
    virtual std::deque<std::shared_ptr<node> >::iterator end() override;
    virtual std::deque<std::shared_ptr<node> >::const_iterator cbegin() const override;
    virtual std::deque<std::shared_ptr<node> >::const_iterator cend() const override;

    virtual std::shared_ptr<container_node> find_container(std::uint32_t id) const override;

private:
    std::deque<std::shared_ptr<node> > m_nodes;
};

std::ostream & operator<<(std::ostream & out, parallel_node const & p);

} // namespace cvpg::imageproc::scripting::detail

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_PARALLEL_NODE_HPP
