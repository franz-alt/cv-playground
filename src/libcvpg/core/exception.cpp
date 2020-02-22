#include <libcvpg/core/exception.hpp>

namespace cvpg {

io_exception::io_exception(std::string message)
    : std::exception()
    , m_message(std::move(message))
{}

const char * io_exception::what() const throw()
{
    return m_message.c_str();
}

} // namespace cvpg
