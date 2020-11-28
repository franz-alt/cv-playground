#include <libcvpg/imageproc/scripting/detail/handler.hpp>

#include <libcvpg/imageproc/scripting/image_processor.hpp>

namespace cvpg { namespace imageproc { namespace scripting { namespace detail {

handler::handler()
    : m_callback()
{}

handler::handler(callback_type callback)
    : m_callback(std::move(callback))
{}

bool handler::is_valid() const
{
    return !!m_callback;
}

handler::result_type handler::operator()(argument_type argument)
{
    return m_callback(std::move(argument));
}

}}}} // namespace cvpg::imageproc::scripting::detail
