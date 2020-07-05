#include <libcvpg/core/exception.hpp>

namespace cvpg {

exception::exception(std::string message)
    : std::exception()
    , m_message(std::move(message))
{}

const char * exception::what() const throw()
{
    return m_message.c_str();
}

invalid_parameter_exception::invalid_parameter_exception(std::string message)
    : std::exception()
    , m_message(std::move(message))
{}

const char * invalid_parameter_exception::what() const throw()
{
    return m_message.c_str();
}

io_exception::io_exception(std::string message)
    : std::exception()
    , m_message(std::move(message))
{}

const char * io_exception::what() const throw()
{
    return m_message.c_str();
}

} // namespace cvpg
