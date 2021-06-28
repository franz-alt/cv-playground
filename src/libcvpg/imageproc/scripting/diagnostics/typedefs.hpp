// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_DIAGNOSTICS_TYPEDEFS_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_DIAGNOSTICS_TYPEDEFS_HPP

#include <list>
#include <map>
#include <string>

#include <boost/asynchronous/diagnostics/any_loggable.hpp>

namespace cvpg::imageproc::scripting::diagnostics {

using servant_job = boost::asynchronous::any_loggable;

using diag_type = std::map<std::string, std::list<boost::asynchronous::diagnostic_item> >;

} // namespace cvpg::imageproc::scripting::diagnostics

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_DIAGNOSTICS_TYPEDEFS_HPP
