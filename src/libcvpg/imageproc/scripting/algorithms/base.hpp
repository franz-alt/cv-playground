#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHMS_BASE_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHMS_BASE_HPP

#include <any>
#include <memory>
#include <string>
#include <vector>

#include <libcvpg/imageproc/scripting/algorithms/parameter.hpp>

namespace cvpg { namespace imageproc { namespace scripting {

namespace detail {

class compiler;
class parser;

} // namespace detail

namespace algorithms {

//
// Base class for all algorithms that should be used at scripting module.
//
// For each algorithm the class contains:
// - metadata (e.g.: name, category, parameters, etc.)
// - ...
//
class base
{
public:
    virtual ~base() = default;

    virtual std::string name() const = 0;

    virtual std::string category() const = 0;

    virtual std::vector<parameter::item::item_type> result() const = 0;

    virtual std::vector<std::vector<parameter> > parameters() const = 0;

    virtual std::vector<std::string> check_parameters(std::vector<std::any> parameters) const = 0;

    virtual void on_parse(std::shared_ptr<detail::parser> parser) const = 0;

    virtual void on_compile(std::uint32_t item_id, std::shared_ptr<detail::compiler> compiler) const = 0;
};

}}}} // namespace cvpg::imageproc::scripting::algorithms

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHMS_BASE_HPP
