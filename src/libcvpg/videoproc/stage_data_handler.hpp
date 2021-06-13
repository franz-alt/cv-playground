#ifndef LIBCVPG_VIDEOPROC_STAGE_DATA_HANDLER_HPP
#define LIBCVPG_VIDEOPROC_STAGE_DATA_HANDLER_HPP

#include <cstdint>
#include <functional>
#include <string>
#include <queue>
#include <vector>

#include <libcvpg/videoproc/frame.hpp>

namespace cvpg::videoproc {

//
// A stage data handler is responsible to store outgoing data that is delivered in any order at a
// processing stage and return the next valid data in the correct order to the stage if the next
// stage indicates that new data could be send.
//
// The template type 'T' has to provide a 'number()' function.
//
template<typename T>
class stage_data_handler
{
public:
    stage_data_handler(std::string name,
                       std::size_t max_stored_entries,
                       std::function<void()> trigger_new_data_callback,
                       std::function<std::size_t()> get_deliver_amount_callback,
                       std::function<void(std::vector<T>, std::function<void()>)> deliver_data_callback,
                       std::function<void()> buffer_full_callback);

    stage_data_handler(stage_data_handler const &) = delete;
    stage_data_handler(stage_data_handler &&) = default;

    stage_data_handler & operator=(stage_data_handler const &) = delete;
    stage_data_handler & operator=(stage_data_handler &&) = default;

    ~stage_data_handler() = default;

    void add(T && t);
    void add(std::vector<T> && t);

    void try_flush();

    bool empty() const;

    bool full() const;

    std::size_t free() const;

private:
    void try_process_input();
    void flush_output();

    std::string m_name;

    std::size_t m_max_stored_entries;

    std::function<void()> m_trigger_new_data_callback;
    std::function<std::size_t()> m_get_deliver_amount_callback;
    std::function<void(std::vector<T>, std::function<void()>)> m_deliver_data_callback;
    std::function<void()> m_buffer_full_callback;

    std::priority_queue<T, std::vector<T>, std::greater<T> > m_in_data;
    std::vector<T> m_out_data;

    std::size_t m_next;
};

// suppress automatic instantiation of stage_data_handler<> for some types
extern template class stage_data_handler<videoproc::frame<image_gray_8bit> >;
extern template class stage_data_handler<videoproc::frame<image_rgb_8bit> >;

} // namespace cvpg::videoproc

#endif // LIBCVPG_VIDEOPROC_STAGE_DATA_HANDLER_HPP
