#ifndef LIBCVPG_VIDEOPROC_PROCESSORS_INTERFRAME_HPP
#define LIBCVPG_VIDEOPROC_PROCESSORS_INTERFRAME_HPP

#include <any>
#include <functional>
#include <map>
#include <memory>
#include <string>

#include <boost/asynchronous/servant_proxy.hpp>
#include <boost/asynchronous/trackable_servant.hpp>

#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/scripting/image_processor.hpp>
#include <libcvpg/imageproc/scripting/diagnostics/typedefs.hpp>
#include <libcvpg/videoproc/frame.hpp>
#include <libcvpg/videoproc/packet.hpp>

namespace cvpg::videoproc::processors {

//
// An inter-frame processor handles subsequent images inside a video stream that was previously
// processed by a frame processor.
//
template<typename Image>
class interframe : public boost::asynchronous::trackable_servant<imageproc::scripting::diagnostics::servant_job, imageproc::scripting::diagnostics::servant_job>
{
public:
    interframe(boost::asynchronous::any_weak_scheduler<imageproc::scripting::diagnostics::servant_job> scheduler,
               boost::asynchronous::any_shared_scheduler_proxy<imageproc::scripting::diagnostics::servant_job> pool,
               std::size_t buffered_packets,
               imageproc::scripting::image_processor_proxy image_processor);

    interframe(interframe const &) = delete;
    interframe(interframe &&) = delete;

    interframe & operator=(interframe const &) = delete;
    interframe & operator=(interframe &&) = delete;

    virtual ~interframe() = default;

    void init(std::size_t context_id,
              std::string script,
              std::function<void(std::size_t)> init_done_callback,
              std::function<void(std::size_t, std::map<std::string, std::any>)> params_callback,
              std::function<void(std::size_t, videoproc::packet<videoproc::frame<Image> >)> packet_callback,
              std::function<void(std::size_t)> next_callback,
              std::function<void(std::size_t)> done_callback);

    void params(std::size_t context_id, std::map<std::string, std::any> p);

    void start(std::size_t context_id);

    void finish(std::size_t context_id);

    void process(std::size_t context_id, videoproc::packet<videoproc::frame<Image> > packet);

    void next(std::size_t context_id);

private:
    void try_process_input(std::size_t context_id);
    void try_flush_buffer(std::size_t context_id);

    std::size_t m_buffered_packets;

    std::shared_ptr<imageproc::scripting::image_processor_proxy> m_image_processor;

    struct processing_context;
    std::map<std::size_t, std::shared_ptr<processing_context> > m_contexts;
};

// suppress automatic instantiation of interframe<> for some types
extern template class interframe<cvpg::image_gray_8bit>;
extern template class interframe<cvpg::image_rgb_8bit>;

//
// Hint: Boost.Asynchronous does not support templated proxies. Becaues the servant itself could
// have template parameters we have to create a proxy for each wanted type.
//

struct image_gray_8bit_interframe_proxy : public boost::asynchronous::servant_proxy<image_gray_8bit_interframe_proxy, interframe<cvpg::image_gray_8bit>, imageproc::scripting::diagnostics::servant_job>
{
   template<typename... Args>
   image_gray_8bit_interframe_proxy(Args... args)
       : boost::asynchronous::servant_proxy<image_gray_8bit_interframe_proxy, interframe<cvpg::image_gray_8bit>, imageproc::scripting::diagnostics::servant_job>(std::forward<Args>(args)...)
   {}

   BOOST_ASYNC_POST_MEMBER_LOG(init, "init", 1)
   BOOST_ASYNC_POST_MEMBER_LOG(params, "params", 1)
   BOOST_ASYNC_POST_MEMBER_LOG(start, "start", 1)
   BOOST_ASYNC_POST_MEMBER_LOG(finish, "finish", 1)
   BOOST_ASYNC_POST_MEMBER_LOG(process, "process", 1)
   BOOST_ASYNC_POST_MEMBER_LOG(next, "next", 1)
};

struct image_rgb_8bit_interframe_proxy : public boost::asynchronous::servant_proxy<image_rgb_8bit_interframe_proxy, interframe<cvpg::image_rgb_8bit>, imageproc::scripting::diagnostics::servant_job>
{
   template<typename... Args>
   image_rgb_8bit_interframe_proxy(Args... args)
       : boost::asynchronous::servant_proxy<image_rgb_8bit_interframe_proxy, interframe<cvpg::image_rgb_8bit>, imageproc::scripting::diagnostics::servant_job>(std::forward<Args>(args)...)
   {}

   BOOST_ASYNC_POST_MEMBER_LOG(init, "init", 1)
   BOOST_ASYNC_POST_MEMBER_LOG(params, "params", 1)
   BOOST_ASYNC_POST_MEMBER_LOG(start, "start", 1)
   BOOST_ASYNC_POST_MEMBER_LOG(finish, "finish", 1)
   BOOST_ASYNC_POST_MEMBER_LOG(process, "process", 1)
   BOOST_ASYNC_POST_MEMBER_LOG(next, "next", 1)
};

} // namespace cvpg::videoproc::processors

#endif // LIBCVPG_VIDEOPROC_PROCESSORS_INTERFRAME_HPP
