#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHMS_HOG_IMAGE_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHMS_HOG_IMAGE_HPP

#include <libcvpg/imageproc/scripting/algorithms/base.hpp>

namespace cvpg::imageproc::scripting::algorithms {

class hog_image : public base
{
public:
    virtual ~hog_image() override = default;

    virtual std::string name() const override;

    virtual std::string category() const override;

    virtual std::vector<scripting::item::types> result() const override;

    virtual parameter_set parameters() const override;

    virtual void on_parse(std::shared_ptr<detail::parser> parser) const override;

    virtual void on_compile(std::uint32_t item_id, std::shared_ptr<detail::compiler> compiler) const override;
};

} // namespace cvpg::imageproc::scripting::algorithms

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHMS_HOG_IMAGE_HPP
