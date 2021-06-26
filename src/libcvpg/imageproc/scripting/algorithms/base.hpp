// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHMS_BASE_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHMS_BASE_HPP

#include <any>
#include <memory>
#include <string>
#include <vector>

#include <libcvpg/imageproc/scripting/algorithms/parameter_set.hpp>

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

    virtual std::vector<scripting::item::types> result() const = 0;

    virtual parameter_set parameters() const = 0;

    virtual void on_parse(std::shared_ptr<detail::parser> parser) const = 0;

    virtual void on_compile(std::uint32_t item_id, std::shared_ptr<detail::compiler> compiler) const = 0;
};

}}}} // namespace cvpg::imageproc::scripting::algorithms

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHMS_BASE_HPP
