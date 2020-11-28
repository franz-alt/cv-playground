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
#include <libcvpg/videoproc/stage_parameters.hpp>
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
    file(boost::asynchronous::any_weak_scheduler<imageproc::scripting::diagnostics::servant_job> scheduler, std::size_t max_frames_read_buffer);

    file(file const &) = delete;
    file(file &&) = delete;

    file & operator=(file const &) = delete;
    file & operator=(file &&) = delete;

    virtual ~file() = default;

    void init(std::size_t context_id, std::string uri, stage_callbacks<Image> callbacks);

    void params(std::size_t context_id, std::map<std::string, std::any> p);

    void start(std::size_t context_id);

    void finish(std::size_t context_id);

    void process(std::size_t context_id, videoproc::packet<videoproc::frame<Image> > packet);

    void next(std::size_t context_id, std::size_t max_new_data);

private:    
    // amount of frames that will be (tried to) read from video file at once
    std::size_t m_max_frames_read_buffer;

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
    using image_type = cvpg::image_gray_8bit;

    template<typename... Args>
    image_gray_8bit_file_proxy(Args... args)
        : boost::asynchronous::servant_proxy<image_gray_8bit_file_proxy, file<cvpg::image_gray_8bit>, imageproc::scripting::diagnostics::servant_job>(std::forward<Args>(args)...)
    {}

    BOOST_ASYNC_POST_MEMBER_LOG(init, "init", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(params, "params", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(start, "start", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(finish, "finish", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(process, "process", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(next, "next", 1)
};

struct image_rgb_8bit_file_proxy : public boost::asynchronous::servant_proxy<image_rgb_8bit_file_proxy, file<cvpg::image_rgb_8bit>, imageproc::scripting::diagnostics::servant_job>
{
    using image_type = cvpg::image_rgb_8bit;

    template<typename... Args>
    image_rgb_8bit_file_proxy(Args... args)
        : boost::asynchronous::servant_proxy<image_rgb_8bit_file_proxy, file<cvpg::image_rgb_8bit>, imageproc::scripting::diagnostics::servant_job>(std::forward<Args>(args)...)
    {}

    BOOST_ASYNC_POST_MEMBER_LOG(init, "init", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(params, "params", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(start, "start", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(finish, "finish", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(process, "process", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(next, "next", 1)
};

} // namespace cvpg::videoproc::sources

#endif // LIBCVPG_VIDEOPROC_SOURCES_FILE_HPP
