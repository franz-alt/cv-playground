#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_ITEM_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_ITEM_HPP

#include <any>
#include <cstdint>
#include <ostream>

namespace cvpg { namespace imageproc { namespace scripting {

//
// An item represents an arbitrary type inside a script.
//
// Allowed types could be:
// - images
// - masks
// - IDs
// - numbers
// - strings
//
class item
{
public:
    enum class types : std::uint8_t
    {
        invalid,
        grayscale_8_bit_image,
        grayscale_8_bit_id,
        rgb_8_bit_image,
        rgb_8_bit_id,
        binary_mask,
        binary_mask_id,
        unsigned_integer,
        signed_integer,
        real,
        boolean,
        characters
    };

    item(types item = types::invalid, std::any value = std::any());

    types type() const;

    std::any value() const;

private:
    types m_type;

    std::any m_value;
};

std::ostream & operator<<(std::ostream & out, item::types const & type);
std::ostream & operator<<(std::ostream & out, item const & i);

}}} // namespace cvpg::imageproc::scripting

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_ITEM_HPP
