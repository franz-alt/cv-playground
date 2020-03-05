#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHMS_PARAMETER_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHMS_PARAMETER_HPP

#include <any>
#include <cstdint>
#include <functional>
#include <ostream>
#include <string>

namespace cvpg { namespace imageproc { namespace scripting { namespace algorithms {

class parameter
{
public:
    struct item
    {
        enum class item_type : std::uint8_t
        {
            invalid,
            grayscale_8_bit_image,
            rgb_8_bit_image,
            unsigned_integer,
            signed_integer,
            real,
            boolean,
            characters
        };

        item_type type = item_type::invalid;

        std::any value = std::any();
    };

    parameter(std::string name = "",
              std::string description = "",
              std::string unit = "",
              item::item_type type = item::item_type::invalid,
              std::function<bool(std::any)> predicate = std::function<bool(std::any)>([](std::any){ return true; }));

    std::string name() const;
    std::string description() const;

    std::string unit() const;

    item::item_type type() const;

    std::function<bool(std::any)> predicate() const;

private:
    std::string m_name;
    std::string m_description;

    std::string m_unit;

    item::item_type m_type;

    std::function<bool(std::any)> m_predicate;
};

std::ostream & operator<<(std::ostream & out, parameter::item::item_type const & p);

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

}}}} // namespace cvpg::imageproc::scripting::algorithms

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHMS_PARAMETER_HPP
