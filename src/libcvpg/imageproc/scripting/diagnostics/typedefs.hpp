#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_DIAGNOSTICS_TYPEDEFS_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_DIAGNOSTICS_TYPEDEFS_HPP

#include <list>
#include <map>
#include <string>

#include <boost/asynchronous/diagnostics/any_loggable.hpp>

namespace cvpg { namespace imageproc { namespace scripting { namespace diagnostics {

using servant_job = boost::asynchronous::any_loggable;

using diag_type = std::map<std::string, std::list<boost::asynchronous::diagnostic_item> >;

}}}} // namespace cvpg::imageproc::scripting::diagnostics

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_DIAGNOSTICS_TYPEDEFS_HPP
