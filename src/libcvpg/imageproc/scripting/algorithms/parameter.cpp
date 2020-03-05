#include <libcvpg/imageproc/scripting/algorithms/parameter.hpp>

namespace cvpg { namespace imageproc { namespace scripting { namespace algorithms {

parameter::parameter(std::string name,
                     std::string description,
                     std::string unit,
                     item::item_type type,
                     std::function<bool(std::any)> predicate)
    : m_name(std::move(name))
    , m_description(std::move(description))
    , m_unit(std::move(unit))
    , m_type(std::move(type))
    , m_predicate(std::move(predicate))
{}

std::string parameter::name() const
{
    return m_name;
}

std::string parameter::description() const
{
    return m_description;
}

std::string parameter::unit() const
{
    return m_unit;
}

parameter::item::item_type parameter::type() const
{
    return m_type;
}

std::function<bool(std::any)> parameter::predicate() const
{
    return m_predicate;
}

std::ostream & operator<<(std::ostream & out, parameter::item::item_type const & t)
{
    switch (t)
    {
        case parameter::item::item_type::invalid:
            out << "invalid";
            break;

        case parameter::item::item_type::grayscale_8_bit_image:
            out << "grayscale 8-bit image";
            break;

        case parameter::item::item_type::rgb_8_bit_image:
            out << "RGB 8-bit image";
            break;

        case parameter::item::item_type::unsigned_integer:
            out << "unsigned integer number";
            break;

        case parameter::item::item_type::signed_integer:
            out << "signed integer number";
            break;

        case parameter::item::item_type::real:
            out << "floating point number";
            break;

        case parameter::item::item_type::boolean:
            out << "boolean";
            break;

        case parameter::item::item_type::characters:
            out << "characters";
            break;
        }

    return out;
}

std::ostream & operator<<(std::ostream & out, parameter const & p)
{
    out << p.name() << " [" << p.type() << "]";

    return out;
}

}}}} // namespace cvpg::imageproc::scripting::algorithms
