// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/imageproc/scripting/item.hpp>

#include <libcvpg/core/image.hpp>

namespace cvpg { namespace imageproc { namespace scripting {

item::item(types item, std::any value)
    : m_type(item)
    , m_value(std::move(value))
{}

item::types item::type() const
{
    return m_type;
}

std::any item::value() const
{
    return m_value;
}

std::ostream & operator<<(std::ostream & out, item::types const & type)
{
    switch (type)
    {
        case item::types::invalid:
            out << "invalid";
            break;

        case item::types::grayscale_8_bit_image:
            out << "grayscale 8-bit image";
            break;

        case item::types::rgb_8_bit_image:
            out << "RGB 8-bit image";
            break;

        case item::types::binary_mask:
            out << "binary mask";
            break;

        case item::types::signed_integer:
            out << "signed integer";
            break;

        case item::types::real:
            out << "real";
            break;

        case item::types::boolean:
            out << "boolean";
            break;

        case item::types::characters:
            out << "characters";
            break;

        case item::types::error:
            out << "error";
            break;
    }

    return out;
}

std::ostream & operator<<(std::ostream & out, item const & i)
{
    const auto t = i.type();

    out << "type='" << t << "',value=";

    try
    {
        switch (t)
        {
            case item::types::invalid:
            {
                out << "N/A";
                break;
            }

            case item::types::grayscale_8_bit_image:
            {
                out << "(" << std::any_cast<image_gray_8bit>(i.value()) << ")";
                break;
            }

            case item::types::rgb_8_bit_image:
            {
                out << "(" << std::any_cast<image_rgb_8bit>(i.value()) << ")";
                break;
            }

            case item::types::binary_mask:
            {
                // TODO implement me
                out << "???";
                break;
            }

            case item::types::signed_integer:
            {
                out << std::any_cast<std::int32_t>(i.value());
                break;
            }

            case item::types::real:
            {
                out << std::any_cast<double>(i.value());
                break;
            }

            case item::types::boolean:
            {
                out << std::any_cast<bool>(i.value());
                break;
            }

            case item::types::characters:
            {
                out << std::any_cast<std::string>(i.value());
                break;
            }

            case item::types::error:
            {
                out << std::any_cast<std::string>(i.value());
                break;
            }
        }
    }
    catch (...)
    {
        out << "N/A";
    }

    return out;
}

}}} // namespace cvpg::imageproc::scripting
