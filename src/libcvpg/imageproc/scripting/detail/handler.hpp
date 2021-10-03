// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_HANDLER_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_HANDLER_HPP

#include <functional>
#include <memory>

#include <boost/asynchronous/detail/continuation_impl.hpp>

namespace cvpg::imageproc::scripting {

class processing_context;

namespace detail {

class handler
{
public:
    using context_type  = cvpg::imageproc::scripting::processing_context;
    using argument_type = std::shared_ptr<context_type>;
    using result_type   = boost::asynchronous::detail::callback_continuation<argument_type>;
    using callback_type = std::function<result_type(argument_type)>;

    // create an invalid handler
    handler();

    handler(callback_type callback);

    handler(handler const &) = default;
    handler(handler &&) = default;

    handler & operator=(handler const &) = default;
    handler & operator=(handler &&) = default;

    bool is_valid() const;

    result_type operator()(argument_type argument);

private:
    callback_type m_callback;
};

} // namespace detail

} // namespace cvpg::imageproc::scripting

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_HANDLER_HPP
