// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/imageproc/scripting/detail/handler.hpp>

#include <libcvpg/imageproc/scripting/image_processor.hpp>

namespace cvpg::imageproc::scripting::detail {

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

} // namespace cvpg::imageproc::scripting::detail
