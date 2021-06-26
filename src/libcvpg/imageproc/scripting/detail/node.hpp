// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_NODE_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_NODE_HPP

#include <functional>
#include <memory>
#include <string>

namespace cvpg { namespace imageproc { namespace scripting { namespace detail {

class node
{
public:
    virtual ~node() = default;

    // process a node
    // virtual void operator()(std::shared_ptr<scope>) = 0;

    // convert to string
    virtual std::string to_string() const = 0;

private:
    std::function<void()> m_callback;
};

}}}} // namespace cvpg::imageproc::scripting::detail

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_NODE_HPP
