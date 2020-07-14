#ifndef LIBCVPG_VIDEOPROC_PIPELINES_FILE_TO_FILE_HPP
#define LIBCVPG_VIDEOPROC_PIPELINES_FILE_TO_FILE_HPP

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <boost/asynchronous/servant_proxy.hpp>
#include <boost/asynchronous/trackable_servant.hpp>
#include <boost/asynchronous/queue/lockfree_queue.hpp>
#include <boost/asynchronous/scheduler_shared_proxy.hpp>
#include <boost/asynchronous/scheduler/single_thread_scheduler.hpp>

#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/scripting/image_processor.hpp>
#include <libcvpg/imageproc/scripting/diagnostics/typedefs.hpp>
#include <libcvpg/videoproc/frame.hpp>
#include <libcvpg/videoproc/packet.hpp>
#include <libcvpg/videoproc/update_indicator.hpp>
#include <libcvpg/videoproc/processors/frame.hpp>
#include <libcvpg/videoproc/processors/interframe.hpp>
#include <libcvpg/videoproc/sinks/file.hpp>
#include <libcvpg/videoproc/sources/file.hpp>

namespace cvpg::videoproc::pipelines {

template<typename Source, typename FrameProcessor, typename InterframeProcessor, typename Sink>
class file_to_file : public boost::asynchronous::trackable_servant<imageproc::scripting::diagnostics::servant_job, imageproc::scripting::diagnostics::servant_job>
{
public:
    file_to_file(boost::asynchronous::any_weak_scheduler<imageproc::scripting::diagnostics::servant_job> scheduler,
                 std::shared_ptr<Source> source,
                 std::shared_ptr<FrameProcessor> frame_processor,
                 std::shared_ptr<InterframeProcessor> interframe_processor,
                 std::shared_ptr<Sink> sink);

    file_to_file(file_to_file const &) = delete;
    file_to_file(file_to_file &&) = delete;

    file_to_file & operator=(file_to_file const &) = delete;
    file_to_file & operator=(file_to_file &&) = delete;

    virtual ~file_to_file() = default;

    void start(std::string input_uri,
               std::string output_uri,
               std::string frame_script,
               std::string inter_frame_script,
               std::function<void()> callback,
               std::function<void(std::size_t, std::int64_t)> init_indicator_callback,
               std::function<void(std::size_t, videoproc::update_indicator)> update_indicator_callback);

private:
    void stage_initialized(std::size_t context_id, std::size_t stage_id);

    std::shared_ptr<Source> m_source;

    std::shared_ptr<FrameProcessor> m_frame_processor;
    std::shared_ptr<InterframeProcessor> m_interframe_processor;

    std::shared_ptr<Sink> m_sink;

    std::size_t m_context_counter;

    std::map<std::size_t, std::vector<std::size_t> > m_stages_initialized;
};

// suppress automatic instantiation of file_to_file<> for some types
extern template class file_to_file<sources::image_gray_8bit_file_proxy, processors::image_gray_8bit_frame_proxy, processors::image_gray_8bit_interframe_proxy, sinks::image_gray_8bit_file_proxy>;
extern template class file_to_file<sources::image_rgb_8bit_file_proxy, processors::image_rgb_8bit_frame_proxy, processors::image_rgb_8bit_interframe_proxy, sinks::image_rgb_8bit_file_proxy>;

//
// Hint: Boost.Asynchronous does not support templated proxies. Becaues the servant itself could
// have template parameters we have to create a proxy for each wanted type.
//

struct image_gray_8bit_file_to_file_proxy : public boost::asynchronous::servant_proxy<
                                                       image_gray_8bit_file_to_file_proxy,
                                                       file_to_file<
                                                           sources::image_gray_8bit_file_proxy,
                                                           processors::image_gray_8bit_frame_proxy,
                                                           processors::image_gray_8bit_interframe_proxy,
                                                           sinks::image_gray_8bit_file_proxy
                                                       >,
                                                       imageproc::scripting::diagnostics::servant_job
                                                   >
{
   template<typename... Args>
   image_gray_8bit_file_to_file_proxy(Args... args)
       : boost::asynchronous::servant_proxy<
             image_gray_8bit_file_to_file_proxy,
             file_to_file<
                 sources::image_gray_8bit_file_proxy,
                 processors::image_gray_8bit_frame_proxy,
                 processors::image_gray_8bit_interframe_proxy,
                 sinks::image_gray_8bit_file_proxy
             >,
             imageproc::scripting::diagnostics::servant_job
         >(std::forward<Args>(args)...)
   {}

   BOOST_ASYNC_POST_MEMBER_LOG(start, "start", 1)
};

struct image_rgb_8bit_file_to_file_proxy : public boost::asynchronous::servant_proxy<
                                                      image_rgb_8bit_file_to_file_proxy,
                                                      file_to_file<
                                                          sources::image_rgb_8bit_file_proxy,
                                                          processors::image_rgb_8bit_frame_proxy,
                                                          processors::image_rgb_8bit_interframe_proxy,
                                                          sinks::image_rgb_8bit_file_proxy
                                                      >,
                                                      imageproc::scripting::diagnostics::servant_job
                                                  >
{
   template<typename... Args>
   image_rgb_8bit_file_to_file_proxy(Args... args)
       : boost::asynchronous::servant_proxy<
             image_rgb_8bit_file_to_file_proxy,
             file_to_file<
                 sources::image_rgb_8bit_file_proxy,
                 processors::image_rgb_8bit_frame_proxy,
                 processors::image_rgb_8bit_interframe_proxy,
                 sinks::image_rgb_8bit_file_proxy
             >,
             imageproc::scripting::diagnostics::servant_job
         >(std::forward<Args>(args)...)
   {}

   BOOST_ASYNC_POST_MEMBER_LOG(start, "start", 1)
};

} // cvpg::videoproc::pipelines

#endif // LIBCVPG_VIDEOPROC_PIPELINES_FILE_TO_FILE_HPP
