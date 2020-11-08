#include <libcvpg/videoproc/stage_data_handler.hpp>

#include <algorithm>
#include <iterator>

namespace cvpg::videoproc {

template<typename T> stage_data_handler<T>::stage_data_handler(std::string name,
                                                               std::size_t max_stored_entries,
                                                               std::function<void()> trigger_new_data_callback,
                                                               std::function<std::size_t()> get_deliver_amount_callback,
                                                               std::function<void(std::vector<T>, std::function<void()>)> deliver_data_callback,
                                                               std::function<void()> buffer_full_callback)
    : m_name(std::move(name))
    , m_max_stored_entries(max_stored_entries)
    , m_trigger_new_data_callback(std::move(trigger_new_data_callback))
    , m_get_deliver_amount_callback(std::move(get_deliver_amount_callback))
    , m_deliver_data_callback(std::move(deliver_data_callback))
    , m_buffer_full_callback(std::move(buffer_full_callback))
    , m_in_data()
    , m_out_data()
    , m_next(0)
{
    // reserve space for output buffer for the same size as input buffer
    m_out_data.reserve(max_stored_entries);
}

template<typename T> void stage_data_handler<T>::add(T t)
{
    if (m_in_data.size() > m_max_stored_entries)
    {
        // TODO temporary disabled!
        // m_buffer_full_callback();
    }

    m_in_data.push(std::move(t));

    try_flush();
}

template<typename T> void stage_data_handler<T>::add(std::vector<T> t)
{
    if ((m_in_data.size() + t.size()) > m_max_stored_entries)
    {
        // TODO temporary disabled!
        // m_buffer_full_callback();
    }

    for (auto & d : t)
    {
        m_in_data.push(std::move(d));
    }

    try_flush();
}

template<typename T> void stage_data_handler<T>::try_process_input()
{
    bool found = true;

    while (found)
    {
        found = !m_in_data.empty() && m_in_data.top().number() == m_next;

        if (found)
        {
            m_out_data.push_back(m_in_data.top());

            m_in_data.pop();

            ++m_next;
        }
    }
}

template<typename T> void stage_data_handler<T>::flush_output()
{
    const std::size_t max_deliver = m_get_deliver_amount_callback();

    if (max_deliver >= m_out_data.size())
    {
        if (!m_out_data.empty())
        {
            // deliver output buffer completly
            m_deliver_data_callback(std::move(m_out_data), m_trigger_new_data_callback);
        }
    }
    else
    {
        if (max_deliver > 0)
        {
            // deliver only 'max_deliver' entries from the beginning of the output buffer
            decltype(m_out_data) moved;
            moved.reserve(max_deliver);

            std::move(m_out_data.begin(), m_out_data.begin() + max_deliver, std::back_inserter(moved));

            m_out_data.erase(m_out_data.begin(), m_out_data.begin() + max_deliver);

            m_deliver_data_callback(std::move(moved), m_trigger_new_data_callback);
        }
    }
}

template<typename T> void stage_data_handler<T>::try_flush()
{
    if (m_in_data.empty())
    {
        // m_trigger_new_data_callback();
    }
    else
    {
        try_process_input();

        const std::size_t max_deliver = m_get_deliver_amount_callback();

        if (max_deliver == 0)
        {
            // don't deliver data if next stage isn't ready to receive new data or no data available at output buffer
            return;
        }

        if (m_out_data.empty())
        {
            m_trigger_new_data_callback();
        }
    }

    flush_output();
}

template<typename T> bool stage_data_handler<T>::empty() const
{
    return m_in_data.empty();
}

template<typename T> bool stage_data_handler<T>::full() const
{
    return m_in_data.size() >= m_max_stored_entries;
}

template<typename T> std::size_t stage_data_handler<T>::free() const
{
    return (m_out_data.size() > (2 * m_max_stored_entries)) ? 0
                                                            : (m_max_stored_entries > m_in_data.size()) ? (m_max_stored_entries - m_in_data.size()) : 0;
}

// manual instantiation of stage_data_handler<> for some types
template class stage_data_handler<videoproc::frame<image_gray_8bit> >;
template class stage_data_handler<videoproc::frame<image_rgb_8bit> >;

} // namespace cvpg::videoproc
