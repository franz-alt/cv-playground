#ifndef LIBCVPG_VIDEOPROC_PIPELINES_ANY_PIPELINE_HPP
#define LIBCVPG_VIDEOPROC_PIPELINES_ANY_PIPELINE_HPP

#include <cstdint>
#include <functional>
#include <string>

#include <boost/asynchronous/detail/any_pointer.hpp>

#include <boost/mpl/vector.hpp>

#include <boost/type_erasure/any.hpp>
#include <boost/type_erasure/constructible.hpp>
#include <boost/type_erasure/member.hpp>
#include <boost/type_erasure/relaxed.hpp>
#include <boost/type_erasure/same_type.hpp>

#include <libcvpg/videoproc/pipelines/parameters.hpp>

BOOST_TYPE_ERASURE_MEMBER((cvpg::videoproc::pipelines)(pipeline_has_start), start, 3);

namespace cvpg::videoproc::pipelines {

typedef boost::mpl::vector5<
    boost::asynchronous::pointer<>,
    boost::type_erasure::same_type<boost::asynchronous::pointer<>::element_type, boost::type_erasure::_a>,
    boost::type_erasure::relaxed,
    boost::type_erasure::copy_constructible<>,
    pipeline_has_start<void(parameters::uris, parameters::scripts, parameters::callbacks), boost::type_erasure::_a>
> any_pipeline_concept;

typedef boost::type_erasure::any<any_pipeline_concept> any_pipeline;

} // cvpg::videoproc::pipelines

#endif // LIBCVPG_VIDEOPROC_PIPELINES_ANY_PIPELINE_HPP
