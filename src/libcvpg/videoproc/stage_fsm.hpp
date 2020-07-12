#ifndef LIBCVPG_VIDEOPROC_STAGE_FSM_HPP
#define LIBCVPG_VIDEOPROC_STAGE_FSM_HPP

#include <functional>
#include <memory>
#include <string>

namespace cvpg::videoproc {

class stage_fsm
{
public:
    stage_fsm(std::string name);

    enum class state_type
    {
        initializing,
        waiting_for_data,
        processing_data
    };

    void on_done(state_type state, std::function<void()> callback);

    enum class event_type
    {
        initialize_done,
        process_data,
        process_data_done
    };

    void process(event_type event);

private:
    struct fsm_def;
    struct fsm;
    std::shared_ptr<fsm> m_fsm;
};

} // namespace cvpg::videoproc

#endif // LIBCVPG_VIDEOPROC_STAGE_FSM_HPP
