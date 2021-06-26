// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_CORE_EXCEPTION_HPP
#define LIBCVPG_CORE_EXCEPTION_HPP

#include <exception>
#include <string>

namespace cvpg {

class exception : public std::exception
{
public:
    exception(std::string message);

    virtual const char * what() const throw() override;

private:
    std::string m_message;
};

class invalid_parameter_exception : public std::exception
{
public:
    invalid_parameter_exception(std::string message);

    virtual const char * what() const throw() override;

private:
    std::string m_message;
};

class io_exception : public std::exception
{
public:
    io_exception(std::string message);

    virtual const char * what() const throw() override;

private:
    std::string m_message;
};

} // namespace cvpg

#endif // LIBCVPG_CORE_EXCEPTION_HPP
