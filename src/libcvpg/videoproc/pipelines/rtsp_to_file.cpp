#include <libcvpg/videoproc/pipelines/rtsp_to_file.hpp>

#include <boost/range/adaptor/reversed.hpp>

#include <libcvpg/videoproc/stage_parameters.hpp>
#include <libcvpg/videoproc/update_indicator.hpp>

namespace cvpg::videoproc::pipelines {

template<typename Stage>
void rtsp_to_file<Stage>::start(std::vector<std::string> stage_parameters, parameters::callbacks callbacks)
{
    if (stage_parameters.size() != m_stages.size())
    {
        // TODO handle error
    }

    auto context_id = ++m_context_counter;

    for (std::size_t i = 0; i < m_stages.size(); ++i)
    {
        auto & current_stage = m_stages[i];

        const bool has_prev_stage = i > 0;
        const bool has_next_stage = i < (m_stages.size() - 1);

        (*current_stage).init(
            context_id,
            stage_parameters[i],
            stage_callbacks<typename Stage::image_type>(
            {
                // init callback
                make_safe_callback(
                    [this, i, callback = (i == 0) ? callbacks.init : [](std::size_t, std::int64_t){}](std::size_t context_id, std::int64_t frames)
                    {
                        callback(context_id, frames);

                        stage_initialized(context_id, i + 1);
                    },
                    "file_to_file::init_done_callback",
                    1
                ),
                // parameters callback
                [next_stage = has_next_stage ? &(m_stages[i + 1]) : nullptr](std::size_t context_id, std::map<std::string, std::any> params)
                {
                    if (!!next_stage)
                    {
                        (**next_stage).params(context_id, std::move(params));
                    }
                },
                // deliver callback
                [next_stage = has_next_stage ? &(m_stages[i + 1]) : nullptr](std::size_t context_id, auto packet)
                {
                    if (!!next_stage)
                    {
                        (**next_stage).process(context_id, std::move(packet));
                    }
                },
                // next callback
                [prev_stage = has_prev_stage ? &(m_stages[i - 1]) : nullptr](std::size_t context_id, std::size_t max_new_data)
                {
                    if (!!prev_stage)
                    {
                        (**prev_stage).next(context_id, max_new_data);
                    }
                },
                // finished callback
                [next_stage = has_next_stage ? &(m_stages[i + 1]) : nullptr](std::size_t context_id)
                {
                    if (!!next_stage)
                    {
                        (**next_stage).finish(context_id);
                    }
                },
                // failed callback
                [callback = callbacks.failed](std::size_t context_id, std::string error)
                {
                    callback(context_id, std::move(error));
                },
                // update callback
                [callback = callbacks.update](std::size_t context_id, update_indicator update)
                {
                    callback(context_id, std::move(update));
                }
            })
        );
    }
}

template<typename Stage>
void rtsp_to_file<Stage>::stage_initialized(std::size_t context_id, std::size_t stage_id)
{
    m_stages_initialized[context_id].push_back(stage_id);

    if (m_stages_initialized[context_id].size() == m_stages.size())
    {
        // start all stages in reverse order
        for (auto & stage : boost::adaptors::reverse(m_stages))
        {
            (*stage).start(context_id);
        }
    }
}

// manual instantation of rtsp_to_file<> for some types
template class rtsp_to_file<any_stage<image_gray_8bit> >;
template class rtsp_to_file<any_stage<image_rgb_8bit> >;

} // cvpg::videoproc::pipelines
