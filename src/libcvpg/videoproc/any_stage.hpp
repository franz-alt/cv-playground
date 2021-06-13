#ifndef LIBCVPG_VIDEOPROC_ANY_STAGE_HPP
#define LIBCVPG_VIDEOPROC_ANY_STAGE_HPP

#include <any>
#include <map>
#include <string>

#include <boost/mpl/vector.hpp>

#include <boost/type_erasure/any.hpp>
#include <boost/type_erasure/constructible.hpp>
#include <boost/type_erasure/member.hpp>
#include <boost/type_erasure/relaxed.hpp>
#include <boost/type_erasure/same_type.hpp>

#include <libcvpg/videoproc/frame.hpp>
#include <libcvpg/videoproc/packet.hpp>
#include <libcvpg/videoproc/stage_parameters.hpp>

BOOST_TYPE_ERASURE_MEMBER((cvpg::videoproc)(sources_has_init), init, 3);
BOOST_TYPE_ERASURE_MEMBER((cvpg::videoproc)(sources_has_params), params, 2);
BOOST_TYPE_ERASURE_MEMBER((cvpg::videoproc)(sources_has_start), start, 1);
BOOST_TYPE_ERASURE_MEMBER((cvpg::videoproc)(sources_has_finish), finish, 1);
BOOST_TYPE_ERASURE_MEMBER((cvpg::videoproc)(sources_has_process), process, 2);
BOOST_TYPE_ERASURE_MEMBER((cvpg::videoproc)(sources_has_next), next, 2);

namespace cvpg::videoproc {

template<class T>
struct any_stage_impl : boost::mpl::vector10<
    boost::asynchronous::pointer<>,
    boost::type_erasure::same_type<boost::asynchronous::pointer<>::element_type, boost::type_erasure::_a>,
    boost::type_erasure::relaxed,
    boost::type_erasure::copy_constructible<>,
    sources_has_init<void(std::size_t, std::string, stage_callbacks<T>), boost::type_erasure::_a>,
    sources_has_params<void(std::size_t, std::map<std::string, std::any>), boost::type_erasure::_a>,
    sources_has_start<void(std::size_t), boost::type_erasure::_a>,
    sources_has_finish<void(std::size_t), boost::type_erasure::_a>,
    sources_has_process<void(std::size_t, cvpg::videoproc::packet<cvpg::videoproc::frame<T> > &&), boost::type_erasure::_a>,
    sources_has_next<void(std::size_t, std::size_t), boost::type_erasure::_a>
> {};

template<class T>
struct any_stage : boost::type_erasure::any<any_stage_impl<T> >
{
    using image_type = T;

    any_stage() : boost::type_erasure::any<any_stage_impl<T> >() {}

    template<class U>
    any_stage(U const & u) : boost::type_erasure::any<any_stage_impl<T> >(u) {}
};

} // namespace cvpg::videoproc

#endif // LIBCVPG_VIDEOPROC_ANY_STAGE_HPP
