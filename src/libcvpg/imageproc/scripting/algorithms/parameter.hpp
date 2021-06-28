// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHMS_PARAMETER_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHMS_PARAMETER_HPP

#include <algorithm>
#include <any>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <ostream>
#include <string>
#include <vector>

#include <libcvpg/imageproc/scripting/item.hpp>

namespace cvpg::imageproc::scripting::algorithms {

class parameter
{
public:
    enum class range_type
    {
        unknown,
        set,
        min_max,
        all
    };

    parameter(std::string name,
              std::string description,
              std::string unit,
              std::initializer_list<scripting::item::types> types,
              std::function<bool(std::any)> predicate = std::function<bool(std::any)>([](std::any){ return true; }));

    parameter(std::string name,
              std::string description,
              std::string unit,
              scripting::item::types type,
              std::any constant_value,
              std::function<bool(std::any)> predicate = std::function<bool(std::any)>([](std::any){ return true; }));

    parameter(std::string name,
              std::string description,
              std::string unit,
              scripting::item::types type,
              std::initializer_list<std::any> value_set = {},
              std::function<bool(std::any)> predicate = std::function<bool(std::any)>([](std::any){ return true; }));

    parameter(std::string name,
              std::string description,
              std::string unit,
              scripting::item::types type,
              std::any min_value,
              std::any max_value,
              std::any value_step_size,
              std::function<bool(std::any)> predicate = std::function<bool(std::any)>([](std::any){ return true; }));

    std::string name() const;
    std::string description() const;

    std::string unit() const;

    std::vector<scripting::item::types> types() const;

    range_type range() const;

    std::vector<std::any> const & values() const;

    std::any min_value() const;
    std::any max_value() const;
    std::any value_step_size() const;

    std::function<bool(std::any)> predicate() const;

    bool is_valid(std::int32_t value) const;
    bool is_valid(double value) const;
    bool is_valid(bool value) const;
    bool is_valid(std::string value) const;

    static std::string to_string(scripting::item::types type, std::any value);

private:
    std::string m_name;
    std::string m_description;

    std::string m_unit;

    std::vector<scripting::item::types> m_types;

    struct range_of_values
    {
        range_type range = range_type::unknown;

        std::vector<std::any> values;

        std::any min_value;
        std::any max_value;
        std::any value_step_size;
    };

    range_of_values m_range;

    std::function<bool(std::any)> m_predicate;
};

// std::ostream & operator<<(std::ostream & out, scripting::item::types const & p);

std::ostream & operator<<(std::ostream & out, parameter const & p);

// unary predicate returns true if element is inside the range [min_value, max_value]
template<typename T>
std::function<bool(std::any)> in_range(T min_value, T max_value)
{
    return [min_value, max_value](std::any element)
           {
               T value = std::any_cast<T>(element);

               return value >= min_value && value <= max_value;
           };
}

// unary predicate returns true if element is inside a specified set of values from the same type
template<typename T>
std::function<bool(std::any)> in_set(std::initializer_list<T> values)
{
    return [values = std::move(values)](std::any element)
           {
               T value = std::any_cast<T>(element);

               return std::find_if(values.begin(),
                                   values.end(),
                                   [value](auto const & v){ return v == value; }) != values.end();
           };
}

// unary predicate returns true if element is a specified constant
template<typename T>
std::function<bool(std::any)> constant(T constant_value)
{
    return [constant_value](std::any element)
           {
               T value = std::any_cast<T>(element);

               return value == constant_value;
           };
}

} // namespace cvpg::imageproc::scripting::algorithms

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHMS_PARAMETER_HPP
