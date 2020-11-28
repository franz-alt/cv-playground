#include <libcvpg/videoproc/stage_fsm.hpp>

#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/state_machine_def.hpp>

namespace msm = boost::msm;
namespace mpl = boost::mpl;

using namespace msm::front;

namespace {

std::string trim_type(std::string const & type)
{
    std::size_t pos = type.rfind("::");

    return pos != std::string::npos ? std::string(type.begin() + pos + 2, type.end()) : type;
}

}

namespace cvpg::videoproc {

struct stage_fsm::fsm_def : public msm::front::state_machine_def<stage_fsm::fsm_def>
{
    //
    // events
    //

    struct initialize_done {};

    struct process_data {};

    struct process_data_done {};

    //
    // states
    //

    template<class Derived>
    struct base_state : public msm::front::state<>
    {
        template<class Event, class Fsm>
        void on_entry_ext(Event const &, Fsm &) {}

        template<class Event, class Fsm>
        void on_entry(Event const & event, Fsm & fsm)
        {
            int status;
            std::string state_name = abi::__cxa_demangle(typeid(Derived).name(), 0, 0, &status);
            std::string event_name = abi::__cxa_demangle(typeid(Event).name(), 0, 0, &status);

            static_cast<Derived*>(this)->on_entry_ext(event, fsm);
        }

        template<class Event, class Fsm>
        void on_exit_ext(Event const &, Fsm &) {}

        template<class Event, class Fsm>
        void on_exit(Event const & event, Fsm & fsm)
        {
            int status;
            std::string state_name = abi::__cxa_demangle(typeid(Derived).name(), 0, 0, &status);

            static_cast<Derived*>(this)->on_exit_ext(event, fsm);
        }
    };

    struct initializing : public base_state<initializing>
    {
        template<class Event, class Fsm>
        void on_exit_ext(Event const & event, Fsm & fsm)
        {
            if (!!fsm.m_on_initialized_callback)
            {
                fsm.m_on_initialized_callback();
            }
        }
    };

    struct waiting_for_data : public base_state<waiting_for_data> {};
    struct processing_data : public base_state<processing_data> {};
    struct finished : public base_state<finished> {};

    //
    // transition table
    //

    struct transition_table : mpl::vector3<
        //  +-------------------+-------------------------+-------------------+---------------------+----------------------+
        //    start               event                     next                action                guard
        //  +-------------------+-------------------------+-------------------+---------------------+----------------------+
        Row < initializing      , initialize_done         , waiting_for_data                                              >,
        //  +-------------------+-------------------------+-------------------+---------------------+----------------------+
        Row < waiting_for_data  , process_data            , processing_data                                               >,
        //  +-------------------+-------------------------+-------------------+---------------------+----------------------+
        Row < processing_data   , process_data_done       , waiting_for_data                                              >
        //  +-------------------+-------------------------+-------------------+---------------------+----------------------+
    > {};

    typedef initializing initial_state;

    fsm_def(std::string name)
        : m_name(std::move(name))
        , m_on_initialized_callback()
    {}

    void set_on_initialized(std::function<void()> callback)
    {
        m_on_initialized_callback = std::move(callback);
    }

    template<class Event, class Fsm>
    void no_transition(Event const &, Fsm & fsm, int state)
    {
        int status;
        std::string event_name = abi::__cxa_demangle(typeid(Event).name(), 0, 0, &status);
    }

private:
    std::string m_name;

    std::function<void()> m_on_initialized_callback;
};

struct stage_fsm::fsm : public msm::back::state_machine<stage_fsm::fsm_def>
{
    template<class ... T>
    fsm(T ... args)
        : msm::back::state_machine<stage_fsm::fsm_def>(std::forward<T>(args)...)
    {}

    void on_initialize_done(std::function<void()> callback)
    {
        fsm_def::set_on_initialized(std::move(callback));
    }

    void initialize_done()
    {
        process_event(fsm_def::initialize_done());
    }
};

stage_fsm::stage_fsm(std::string name)
    : m_fsm(std::make_shared<fsm>(std::move(name)))
{
    m_fsm->start();
}

void stage_fsm::on_done(state_type state, std::function<void()> callback)
{
    if (state == state_type::initializing)
    {
        m_fsm->on_initialize_done(std::move(callback));
    }
}

void stage_fsm::process(event_type event)
{
    if (event == event_type::initialize_done)
    {
        m_fsm->initialize_done();
    }
}

} // namespace cvpg::videoproc
