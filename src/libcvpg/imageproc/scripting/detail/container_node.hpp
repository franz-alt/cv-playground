#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_CONTAINER_NODE_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_CONTAINER_NODE_HPP

#include <libcvpg/imageproc/scripting/detail/node.hpp>

#include <cstdint>
#include <deque>
#include <memory>
#include <ostream>
#include <string>

namespace cvpg { namespace imageproc { namespace scripting { namespace detail {

class container_node : public node
{
public:
    virtual ~container_node() override = default;

    // virtual void operator()(std::shared_ptr<scope>) override = 0;

    virtual std::string to_string() const override = 0;

    virtual void push_front(std::shared_ptr<node>) = 0;
    virtual void push_back(std::shared_ptr<node>) = 0;

    virtual std::size_t size() const = 0;

    // return iterators
    virtual std::deque<std::shared_ptr<node> >::iterator begin() = 0;
    virtual std::deque<std::shared_ptr<node> >::iterator end() = 0;
    virtual std::deque<std::shared_ptr<node> >::const_iterator cbegin() const = 0;
    virtual std::deque<std::shared_ptr<node> >::const_iterator cend() const = 0;

    virtual std::shared_ptr<container_node> find_container(std::uint32_t) const = 0;
};

std::ostream & operator<<(std::ostream & out, container_node const & c);

}}}} // namespace cvpg::imageproc::scripting::detail

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_CONTAINER_NODE_HPP
