#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHMS_SOBEL_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHMS_SOBEL_HPP

#include <libcvpg/imageproc/scripting/algorithms/base.hpp>

namespace cvpg { namespace imageproc { namespace scripting { namespace algorithms {

class sobel : public base
{
public:
    virtual ~sobel() override = default;

    virtual std::string name() const override;

    virtual std::string category() const override;

    virtual std::vector<parameter::item::item_type> result() const override;

    virtual std::vector<std::vector<parameter> > parameters() const override;

    virtual std::vector<std::string> check_parameters(std::vector<std::any> parameters) const override;

    virtual void on_parse(std::shared_ptr<detail::parser> parser) const override;

    virtual void on_compile(std::uint32_t item_id, std::shared_ptr<detail::compiler> compiler) const override;
};

}}}} // namespace cvpg::imageproc::scripting::algorithms

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHMS_SOBEL_HPP
