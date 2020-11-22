#include <libcvpg/videoproc/pipelines/rtsp_to_file.hpp>

#include <libcvpg/videoproc/update_indicator.hpp>

namespace cvpg::videoproc::pipelines {

template<typename Source, typename FrameProcessor, typename InterframeProcessor, typename Sink>
rtsp_to_file<Source, FrameProcessor, InterframeProcessor, Sink>::rtsp_to_file(boost::asynchronous::any_weak_scheduler<imageproc::scripting::diagnostics::servant_job> scheduler,
                                                                              std::shared_ptr<Source> source,
                                                                              std::shared_ptr<FrameProcessor> frame_processor,
                                                                              std::shared_ptr<InterframeProcessor> interframe_processor,
                                                                              std::shared_ptr<Sink> sink)
    : boost::asynchronous::trackable_servant<imageproc::scripting::diagnostics::servant_job, imageproc::scripting::diagnostics::servant_job>(scheduler)
    , m_source(source)
    , m_frame_processor(frame_processor)
    , m_interframe_processor(interframe_processor)
    , m_sink(sink)
    , m_context_counter(0)
{}

template<typename Source, typename FrameProcessor, typename InterframeProcessor, typename Sink>
void rtsp_to_file<Source, FrameProcessor, InterframeProcessor, Sink>::start(parameters::uris uris, parameters::scripts scripts, parameters::callbacks callbacks)
{
    auto context_id = ++m_context_counter;

    m_source->init(
        context_id,
        std::move(uris.input),
        // init done callback
        make_safe_callback(
            [this, init_indicator_callback = callbacks.init](std::size_t context_id,  std::int64_t /*frames*/)
            {
                // init update indicator with 0 frames to indicate 'endless'
                init_indicator_callback(context_id, 0);

                stage_initialized(context_id, 1);
            },
            "pipeline::init_done_callback",
            1
        ),
        // params callback
        [frame_processor = m_frame_processor](std::size_t context_id, std::map<std::string, std::any> params)
        {
            frame_processor->params(context_id, std::move(params));
        },
        // packet callback
        [frame_processor = m_frame_processor](std::size_t context_id, auto packet)
        {
            frame_processor->process(context_id, std::move(packet));
        },
        // done/finish callback
        [frame_processor = m_frame_processor](std::size_t context_id)
        {
            frame_processor->finish(context_id);
        },
        // failed callback
        [failed_indicator_callback = callbacks.failed](std::size_t context_id, std::string error)
        {
            failed_indicator_callback(context_id, std::move(error));
        },
        // update indicator
        [update_indicator_callback = callbacks.update](std::size_t context_id, update_indicator update)
        {
            update_indicator_callback(context_id, std::move(update));
        }
    );

    m_frame_processor->init(
        context_id,
        std::move(scripts.frame),
        // init done callback
        make_safe_callback(
            [this](std::size_t context_id)
            {
                stage_initialized(context_id, 2);
            },
            "pipeline::init_done_callback",
            1
        ),
        // params callback
        [interframe_processor = m_interframe_processor](std::size_t context_id, std::map<std::string, std::any> params)
        {
            interframe_processor->params(context_id, std::move(params));
        },
        // packet done callback
        [interframe_processor = m_interframe_processor](std::size_t context_id, auto packet)
        {
            interframe_processor->process(context_id, std::move(packet));
        },
        // next callback
        [source = m_source](std::size_t context_id, std::size_t max_new_data)
        {
            source->next(context_id, max_new_data);
        },
        // done/finish callback
        [interframe_processor = m_interframe_processor](std::size_t context_id)
        {
            interframe_processor->finish(context_id);
        },
        // failed callback
        [failed_indicator_callback = callbacks.failed](std::size_t context_id, std::string error)
        {
            failed_indicator_callback(context_id, std::move(error));
        },
        // update indicator
        [update_indicator_callback = callbacks.update](std::size_t context_id, update_indicator update)
        {
            update_indicator_callback(context_id, std::move(update));
        }
    );

    m_interframe_processor->init(
        context_id,
        std::move(scripts.interframe),
        // init done callback
        make_safe_callback(
            [this](std::size_t context_id)
            {
                stage_initialized(context_id, 3);
            },
            "pipeline::init_done_callback",
            1
        ),
        // params callback
        [sink = m_sink](std::size_t context_id, std::map<std::string, std::any> params)
        {
            sink->params(context_id, std::move(params));
        },
        // packet done callback
        [sink = m_sink](std::size_t context_id, auto packet)
        {
            sink->process(context_id, std::move(packet));
        },
        // next callback
        [frame_processor = m_frame_processor](std::size_t context_id, std::size_t max_new_data)
        {
            frame_processor->next(context_id, max_new_data);
        },
        // done/finish callback
        [sink = m_sink](std::size_t context_id)
        {
            sink->finish(context_id);
        },
        // failed callback
        [failed_indicator_callback = callbacks.failed](std::size_t context_id, std::string error)
        {
            failed_indicator_callback(context_id, std::move(error));
        },
        // update indicator
        [update_indicator_callback = callbacks.update](std::size_t context_id, update_indicator update)
        {
            update_indicator_callback(context_id, std::move(update));
        }
    );

    m_sink->init(
        context_id,
        std::move(uris.output),
        // init done callback
        make_safe_callback(
            [this](std::size_t context_id)
            {
                stage_initialized(context_id, 4);
            },
            "pipeline::init_done_callback",
            1
        ),
        // next callback
        [interframe_processor = m_interframe_processor](std::size_t context_id, std::size_t max_new_data)
        {
            interframe_processor->next(context_id, max_new_data);
        },
        // finished callback
        [this, finished_callback = callbacks.finished](std::size_t context_id)
        {
            finished_callback();
        },
        // failed callback
        [failed_indicator_callback = callbacks.failed](std::size_t context_id, std::string error)
        {
            failed_indicator_callback(context_id, std::move(error));
        },
        // update indicator
        [update_indicator_callback = callbacks.update](std::size_t context_id, update_indicator update)
        {
            update_indicator_callback(context_id, std::move(update));
        }
    );
}

template<typename Source, typename FrameProcessor, typename InterframeProcessor, typename Sink>
void rtsp_to_file<Source, FrameProcessor, InterframeProcessor, Sink>::stage_initialized(std::size_t context_id, std::size_t stage_id)
{
    m_stages_initialized[context_id].push_back(stage_id);

    if (m_stages_initialized[context_id].size() == 4)
    {
        m_sink->start(context_id);
        m_interframe_processor->start(context_id);
        m_frame_processor->start(context_id);
        m_source->start(context_id);
    }
}

// manual instantation of rtsp_to_file<> for some types
template class rtsp_to_file<sources::image_gray_8bit_rtsp_proxy, processors::image_gray_8bit_frame_proxy, processors::image_gray_8bit_interframe_proxy, sinks::image_gray_8bit_file_proxy>;
template class rtsp_to_file<sources::image_rgb_8bit_rtsp_proxy, processors::image_rgb_8bit_frame_proxy, processors::image_rgb_8bit_interframe_proxy, sinks::image_rgb_8bit_file_proxy>;

} // cvpg::videoproc::pipelines
