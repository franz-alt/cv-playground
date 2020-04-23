#include <libcvpg/imageproc/scripting/algorithms/parameter.hpp>

#include <cmath>
#include <sstream>

#include <libcvpg/core/image.hpp>

namespace {

bool is_valid(std::vector<std::any> const & set, std::string value)
{
    try
    {
        for (auto const & v : set)
        {
            if (std::any_cast<std::string>(v) == value)
            {
                return true;
            }
        }
    }
    catch (...)
    {
        // ignore exceptions
    }

    return false;
}

}

namespace cvpg { namespace imageproc { namespace scripting { namespace algorithms {

parameter::parameter(std::string name,
                     std::string description,
                     std::string unit,
                     std::initializer_list<item::item_type> types,
                     std::function<bool(std::any)> predicate)
    : m_name(std::move(name))
    , m_description(std::move(description))
    , m_unit(std::move(unit))
    , m_types(std::move(types))
    , m_range()
    , m_predicate(std::move(predicate))
{}

parameter::parameter(std::string name,
                     std::string description,
                     std::string unit,
                     item::item_type type,
                     std::any constant_value,
                     std::function<bool(std::any)> predicate)
    : m_name(std::move(name))
    , m_description(std::move(description))
    , m_unit(std::move(unit))
    , m_types()
    , m_range()
    , m_predicate(std::move(predicate))
{
    m_types.push_back(std::move(type));

    m_range.range = range_type::set;
    m_range.values.push_back(std::move(constant_value));
}

parameter::parameter(std::string name,
                     std::string description,
                     std::string unit,
                     item::item_type type,
                     std::initializer_list<std::any> value_set,
                     std::function<bool(std::any)> predicate)
    : m_name(std::move(name))
    , m_description(std::move(description))
    , m_unit(std::move(unit))
    , m_types()
    , m_range()
    , m_predicate(std::move(predicate))
{
    m_types.push_back(std::move(type));

    if (value_set.size() != 0)
    {
        m_range.range = range_type::set;

        for (auto const & v : value_set)
        {
            m_range.values.push_back(v);
        }
    }
}

parameter::parameter(std::string name,
                     std::string description,
                     std::string unit,
                     item::item_type type,
                     std::any min_value,
                     std::any max_value,
                     std::any value_step_size,
                     std::function<bool(std::any)> predicate)
    : m_name(std::move(name))
    , m_description(std::move(description))
    , m_unit(std::move(unit))
    , m_types()
    , m_range()
    , m_predicate(std::move(predicate))
{
    m_types.push_back(std::move(type));

    m_range.range = range_type::min_max;
    m_range.min_value = std::move(min_value);
    m_range.max_value = std::move(max_value);
    m_range.value_step_size = std::move(value_step_size);
}

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

std::vector<parameter::item::item_type> parameter::types() const
{
    return m_types;
}

parameter::range_type parameter::range() const
{
    return m_range.range;
}

std::vector<std::any> const & parameter::values() const
{
    return m_range.values;
}

std::any parameter::min_value() const
{
    return m_range.min_value;
}

std::any parameter::max_value() const
{
    return m_range.max_value;
}

std::any parameter::value_step_size() const
{
    return m_range.value_step_size;
}

std::function<bool(std::any)> parameter::predicate() const
{
    return m_predicate;
}

bool parameter::is_valid(std::uint32_t value) const
{
    try
    {
        switch (range())
        {
            default:
            case range_type::unknown:
                // TODO error handling
                break;

            case range_type::set:
            {
                for (auto const & v : values())
                {
                    if (value == std::any_cast<std::uint32_t>(v))
                    {
                        return true;
                    }
                }

                break;
            }

            case range_type::min_max:
            {
                auto val = std::any_cast<std::uint32_t>(value);
                auto min = std::any_cast<std::uint32_t>(min_value());
                auto max = std::any_cast<std::uint32_t>(max_value());
                auto step = std::any_cast<std::uint32_t>(value_step_size());

                if (val >= min && val <= max && ((val - min) % step) == 0 && predicate()(std::move(value)))
                {
                    return true;
                }

                break;
            }
        }
    }
    catch (...)
    {
        // ignore exceptions
    }
    
    return false;
}

bool parameter::is_valid(std::int32_t value) const
{
    try
    {
        switch (range())
        {
            default:
            case range_type::unknown:
                // TODO error handling
                break;

            case range_type::set:
            {
                for (auto const & v : values())
                {
                    if (value == std::any_cast<std::int32_t>(v))
                    {
                        return true;
                    }
                }

                break;
            }

            case range_type::min_max:
            {
                auto val = std::any_cast<std::int32_t>(value);
                auto min = std::any_cast<std::int32_t>(min_value());
                auto max = std::any_cast<std::int32_t>(max_value());
                auto step = std::any_cast<std::int32_t>(value_step_size());

                if (val >= min && val <= max && ((val - min) % step) == 0 && predicate()(std::move(value)))
                {
                    return true;
                }

                break;
            }
        }
    }
    catch (...)
    {
        // ignore exceptions
    }
    
    return false;
}

bool parameter::is_valid(double value) const
{
    try
    {
        switch (range())
        {
            default:
            case range_type::unknown:
                // TODO error handling
                break;

            case range_type::set:
            {
                for (auto const & v : values())
                {
                    if (value == std::any_cast<double>(v))
                    {
                        return true;
                    }
                }

                break;
            }

            case range_type::min_max:
            {
                auto val = std::any_cast<double>(value);
                auto min = std::any_cast<double>(min_value());
                auto max = std::any_cast<double>(max_value());
                auto step = std::any_cast<double>(value_step_size());

                if (val >= min && val <= max && fmod(val - min, step) > 0.0 && predicate()(std::move(value)))
                {
                    return true;
                }

                break;
            }
        }
    }
    catch (...)
    {
        // ignore exceptions
    }
    
    return false;
}

bool parameter::is_valid(bool value) const
{
    try
    {
        for (auto const & v : values())
        {
            if (value == std::any_cast<bool>(v))
            {
                return true;
            }
        }
    }
    catch (...)
    {
        // ignore exceptions
    }
    
    return false;
}

bool parameter::is_valid(std::string value) const
{
    try
    {
        for (auto const & v : values())
        {
            if (value == std::any_cast<std::string>(v))
            {
                return true;
            }
        }
    }
    catch (...)
    {
        // ignore exceptions
    }
    
    return false;
}

std::string parameter::to_string(item::item_type type, std::any value)
{
    std::stringstream ss;

    switch (type)
    {
        default:
            // no output in this case
            break;

        case parameter::item::item_type::unsigned_integer:
            ss << std::any_cast<std::uint32_t>(value);
            break;
        
        case parameter::item::item_type::signed_integer:
            ss << std::any_cast<std::int32_t>(value);
            break;

        case parameter::item::item_type::real:
            ss << std::any_cast<double>(value);
            break;

        case parameter::item::item_type::boolean:
            ss << std::any_cast<bool>(value);
            break;

        case parameter::item::item_type::characters:
            ss << "\"" << std::any_cast<std::string>(value) << "\"";
            break;
    }

    return ss.str();
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
    out << p.name() << " [";

    if (!p.types().empty())
    {
        decltype(p.types()) const & types = p.types();

        out << *(types.begin());

        for (auto it = ++(types.begin()); it != types.end(); ++it)
        {
            out << ", " << *it;
        }

        switch (p.range())
        {
            default:
            case parameter::range_type::unknown:
                break;
            
            case parameter::range_type::set:
            {
                if (!p.values().empty())
                {
                    auto it = p.values().begin();

                    out << " { values: " << parameter::to_string(p.types().front(), *it++);

                    for (; it != p.values().end(); ++it)
                    {
                        out << ", " << parameter::to_string(p.types().front(), *it);
                    }

                    out << " }";
                }

                break;
            }

            case parameter::range_type::min_max:
            {
                out << " { range: [" << parameter::to_string(p.types().front(), p.min_value()) << ".." << parameter::to_string(p.types().front(), p.max_value() ) << "], step: " << parameter::to_string(p.types().front(), p.value_step_size()) << " }";
                break;
            }
        }
    }

    out << "]";

    return out;
}

}}}} // namespace cvpg::imageproc::scripting::algorithms
