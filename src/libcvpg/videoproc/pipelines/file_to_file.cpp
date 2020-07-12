#include <libcvpg/videoproc/pipelines/file_to_file.hpp>

namespace cvpg::videoproc::pipelines {

template<typename Source, typename FrameProcessor, typename InterframeProcessor, typename Sink>
file_to_file<Source, FrameProcessor, InterframeProcessor, Sink>::file_to_file(boost::asynchronous::any_weak_scheduler<imageproc::scripting::diagnostics::servant_job> scheduler,
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
void file_to_file<Source, FrameProcessor, InterframeProcessor, Sink>::start(std::string input_uri,
                                                                            std::string output_uri,
                                                                            std::string frame_script,
                                                                            std::string inter_frame_script,
                                                                            std::function<void()> callback)
{
    auto context_id = ++m_context_counter;

    m_source->init(
        context_id,
        std::move(input_uri),
        // init done callback
        make_safe_callback(
            [this](std::size_t context_id)
            {
                stage_initialized(context_id, 1);
            },
            "pipeline::init_done_callback",
            1
        ),
        // params callback
        [frame_processor = m_frame_processor](std::size_t context_id, std::map<std::string, std::any> params) mutable
        {
            frame_processor->params(context_id, std::move(params));
        },
        // packet callback
        [frame_processor = m_frame_processor](std::size_t context_id, auto packet) mutable
        {
            frame_processor->process(context_id, std::move(packet));
        },
        // done/finish callback
        [frame_processor = m_frame_processor](std::size_t context_id)
        {
            frame_processor->finish(context_id);
        }
    );

    m_frame_processor->init(
        context_id,
        std::move(frame_script),
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
        [interframe_processor = m_interframe_processor](std::size_t context_id, std::map<std::string, std::any> params) mutable
        {
            interframe_processor->params(context_id, std::move(params));
        },
        // packet done callback
        [interframe_processor = m_interframe_processor](std::size_t context_id, auto packet) mutable
        {
            interframe_processor->process(context_id, std::move(packet));
        },
        // next callback
        [source = m_source](std::size_t context_id)
        {
            source->next(context_id);
        },
        // done/finish callback
        [interframe_processor = m_interframe_processor](std::size_t context_id)
        {
            interframe_processor->finish(context_id);
        }
    );

    m_interframe_processor->init(
        context_id,
        std::move(inter_frame_script),
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
        [sink = m_sink](std::size_t context_id, std::map<std::string, std::any> params) mutable
        {
            sink->params(context_id, std::move(params));
        },
        // packet done callback
        [sink = m_sink](std::size_t context_id, auto packet) mutable
        {
            sink->process(context_id, std::move(packet));
        },
        // next callback
        [frame_processor = m_frame_processor](std::size_t context_id)
        {
            frame_processor->next(context_id);
        },
        // done/finish callback
        [sink = m_sink](std::size_t context_id)
        {
            sink->finish(context_id);
        }
    );

    m_sink->init(
        context_id,
        std::move(output_uri),
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
        [interframe_processor = m_interframe_processor](std::size_t context_id)
        {
            interframe_processor->next(context_id);
        },
        // finished callback
        [callback](std::size_t /*context_id*/)
        {
            callback();
        }
    );
}

template<typename Source, typename FrameProcessor, typename InterframeProcessor, typename Sink>
void file_to_file<Source, FrameProcessor, InterframeProcessor, Sink>::stage_initialized(std::size_t context_id, std::size_t stage_id)
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

// manual instantation of file_to_file<> for some types
template class file_to_file<sources::image_gray_8bit_file_proxy, processors::image_gray_8bit_frame_proxy, processors::image_gray_8bit_interframe_proxy, sinks::image_gray_8bit_file_proxy>;
template class file_to_file<sources::image_rgb_8bit_file_proxy, processors::image_rgb_8bit_frame_proxy, processors::image_rgb_8bit_interframe_proxy, sinks::image_rgb_8bit_file_proxy>;

} // cvpg::videoproc::pipelines
