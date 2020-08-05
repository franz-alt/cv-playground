#ifndef LIBCVPG_VIDEOPROC_SOURCES_FILE_HPP
#define LIBCVPG_VIDEOPROC_SOURCES_FILE_HPP

#include <any>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>

#include <boost/asynchronous/servant_proxy.hpp>
#include <boost/asynchronous/trackable_servant.hpp>

#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/scripting/diagnostics/typedefs.hpp>
#include <libcvpg/videoproc/frame.hpp>
#include <libcvpg/videoproc/packet.hpp>
#include <libcvpg/videoproc/update_indicator.hpp>

namespace cvpg::videoproc::sources {

//
// A file source reads a video from a given URI and produces packets containing video frames.
//
// A certain amount of frames will be read from the video file at once and bundled inside a single
// packet. These packets will be stored inside an output buffer. Each packet at the output buffer
// will be delivered to the next stage if this stage is ready to receive new data.
//
template<typename Image>
class file : public boost::asynchronous::trackable_servant<imageproc::scripting::diagnostics::servant_job, imageproc::scripting::diagnostics::servant_job>
{
public:
    file(boost::asynchronous::any_weak_scheduler<imageproc::scripting::diagnostics::servant_job> scheduler,
         std::size_t frames_per_packet,
         std::size_t max_packets_output_buffer);

    file(file const &) = delete;
    file(file &&) = delete;

    file & operator=(file const &) = delete;
    file & operator=(file &&) = delete;

    virtual ~file() = default;

    void init(std::size_t context_id,
              std::string uri,
              std::function<void(std::size_t, std::int64_t)> init_done_callback,
              std::function<void(std::size_t, std::map<std::string, std::any>)> params_callback,
              std::function<void(std::size_t, videoproc::packet<videoproc::frame<Image> >)> packet_callback,
              std::function<void(std::size_t)> done_callback,
              std::function<void(std::size_t, std::string)> failed_callback,
              std::function<void(std::size_t, update_indicator)> update_indicator_callback);

    void start(std::size_t context_id);

    void next(std::size_t context_id);

private:    
    void try_flush_buffer(std::size_t context_id);

    // amount of frames that will be (tried to) read from video file at once
    std::size_t m_frames_per_packet;

    // maximum size of packet send buffer
    std::size_t m_max_packets_output_buffer;

    struct processing_context;
    std::map<std::size_t, std::shared_ptr<processing_context> > m_contexts;
};

// suppress automatic instantiation of file<> for some types
extern template class file<cvpg::image_gray_8bit>;
extern template class file<cvpg::image_rgb_8bit>;

//
// Hint: Boost.Asynchronous does not support templated proxies. Becaues the servant itself could
// have template parameters we have to create a proxy for each wanted type.
//

struct image_gray_8bit_file_proxy : public boost::asynchronous::servant_proxy<image_gray_8bit_file_proxy, file<cvpg::image_gray_8bit>, imageproc::scripting::diagnostics::servant_job>
{
    template<typename... Args>
    image_gray_8bit_file_proxy(Args... args)
        : boost::asynchronous::servant_proxy<image_gray_8bit_file_proxy, file<cvpg::image_gray_8bit>, imageproc::scripting::diagnostics::servant_job>(std::forward<Args>(args)...)
    {}

    BOOST_ASYNC_POST_MEMBER_LOG(init, "init", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(start, "start", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(next, "next", 1)
};

struct image_rgb_8bit_file_proxy : public boost::asynchronous::servant_proxy<image_rgb_8bit_file_proxy, file<cvpg::image_rgb_8bit>, imageproc::scripting::diagnostics::servant_job>
{
    template<typename... Args>
    image_rgb_8bit_file_proxy(Args... args)
        : boost::asynchronous::servant_proxy<image_rgb_8bit_file_proxy, file<cvpg::image_rgb_8bit>, imageproc::scripting::diagnostics::servant_job>(std::forward<Args>(args)...)
    {}

    BOOST_ASYNC_POST_MEMBER_LOG(init, "init", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(start, "start", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(next, "next", 1)
};

} // namespace cvpg::videoproc::sources

#endif // LIBCVPG_VIDEOPROC_SOURCES_FILE_HPP
