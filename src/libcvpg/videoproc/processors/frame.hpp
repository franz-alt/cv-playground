// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_VIDEOPROC_PROCESSORS_FRAME_HPP
#define LIBCVPG_VIDEOPROC_PROCESSORS_FRAME_HPP

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
#include <libcvpg/videoproc/stage_parameters.hpp>
#include <libcvpg/videoproc/update_indicator.hpp>

namespace cvpg::videoproc::processors {

//
// A frame processor handles image processing on single frames inside a video stream.
//
template<typename Image>
class frame : public boost::asynchronous::trackable_servant<imageproc::scripting::diagnostics::servant_job, imageproc::scripting::diagnostics::servant_job>
{
public:
    frame(boost::asynchronous::any_weak_scheduler<imageproc::scripting::diagnostics::servant_job> scheduler,
          std::size_t max_frames_output_buffer,
          imageproc::scripting::image_processor_proxy image_processor);

    frame(frame const &) = delete;
    frame(frame &&) = delete;

    frame & operator=(frame const &) = delete;
    frame & operator=(frame &&) = delete;

    virtual ~frame() = default;

    void init(std::size_t context_id, std::string script, stage_callbacks<Image> callbacks);

    void params(std::size_t context_id, std::map<std::string, std::any> p);

    void start(std::size_t context_id);

    void finish(std::size_t context_id);

    void process(std::size_t context_id, videoproc::packet<videoproc::frame<Image> > && packet);

    void next(std::size_t context_id, std::size_t max_new_data);

private:
    void process_next_frames(std::size_t context_id, std::vector<videoproc::frame<Image> > && frames);

    // maximum amount of frames at output buffer
    std::size_t m_max_frames_output_buffer;

    std::shared_ptr<imageproc::scripting::image_processor_proxy> m_image_processor;

    struct processing_context;
    std::map<std::size_t, std::shared_ptr<processing_context> > m_contexts;
};

// suppress automatic instantiation of frame<> for some types
extern template class frame<cvpg::image_gray_8bit>;
extern template class frame<cvpg::image_rgb_8bit>;

//
// Hint: Boost.Asynchronous does not support templated proxies. Becaues the servant itself could
// have template parameters we have to create a proxy for each wanted type.
//

struct image_gray_8bit_frame_proxy : public boost::asynchronous::servant_proxy<image_gray_8bit_frame_proxy, frame<cvpg::image_gray_8bit>, imageproc::scripting::diagnostics::servant_job>
{
    template<typename... Args>
    image_gray_8bit_frame_proxy(Args... args)
        : boost::asynchronous::servant_proxy<image_gray_8bit_frame_proxy, frame<cvpg::image_gray_8bit>, imageproc::scripting::diagnostics::servant_job>(std::forward<Args>(args)...)
    {}

    BOOST_ASYNC_POST_MEMBER_LOG(init, "init", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(params, "params", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(start, "start", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(finish, "finish", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(process, "process", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(next, "next", 1)
};

struct image_rgb_8bit_frame_proxy : public boost::asynchronous::servant_proxy<image_rgb_8bit_frame_proxy, frame<cvpg::image_rgb_8bit>, imageproc::scripting::diagnostics::servant_job>
{
    template<typename... Args>
    image_rgb_8bit_frame_proxy(Args... args)
        : boost::asynchronous::servant_proxy<image_rgb_8bit_frame_proxy, frame<cvpg::image_rgb_8bit>, imageproc::scripting::diagnostics::servant_job>(std::forward<Args>(args)...)
    {}

    BOOST_ASYNC_POST_MEMBER_LOG(init, "init", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(params, "params", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(start, "start", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(finish, "finish", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(process, "process", 1)
    BOOST_ASYNC_POST_MEMBER_LOG(next, "next", 1)
};

} // namespace cvpg::videoproc::processors

#endif // LIBCVPG_VIDEOPROC_PROCESSORS_FRAME_HPP
