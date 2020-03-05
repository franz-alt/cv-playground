#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_HANDLER_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_HANDLER_HPP

#include <functional>
#include <memory>

#include <boost/asynchronous/detail/continuation_impl.hpp>

namespace cvpg { namespace imageproc { namespace scripting {

class image_processor;

namespace detail {

class handler
{
public:
    using result_type = boost::asynchronous::detail::callback_continuation<std::shared_ptr<image_processor> >;
    using argument_type = std::shared_ptr<image_processor>;
    using callback_type = std::function<result_type(argument_type, std::size_t)>;

    // create an invalid handler
    handler();

    handler(callback_type callback);

    handler(handler const &) = default;
    handler(handler &&) = default;

    handler & operator=(handler const &) = default;
    handler & operator=(handler &&) = default;

    bool is_valid() const;

    result_type operator()(argument_type argument, std::size_t context_id);

private:
    callback_type m_callback;
};

}}}} // namespace cvpg::imageproc::scripting::detail

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_DETAIL_HANDLER_HPP
