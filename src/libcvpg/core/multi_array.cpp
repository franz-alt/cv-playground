#include <libcvpg/core/multi_array.hpp>

#include <stdexcept>

namespace cvpg {

template<class T> multi_array<T>::multi_array(std::initializer_list<int> shape)
    : m_shape(std::move(shape))
{
    m_capacity = std::accumulate(m_shape.cbegin(), m_shape.cend(), 1, std::multiplies<int>());
    m_data.resize(m_capacity);
}

template<class T> multi_array<T>::multi_array(std::vector<int> shape)
    : m_shape(std::move(shape))
{
    m_capacity = std::accumulate(m_shape.cbegin(), m_shape.cend(), 1, std::multiplies<int>());
    m_data.resize(m_capacity);
}

template<class T> std::size_t multi_array<T>::dimensions() const noexcept
{
    return m_shape.size();
}

template<class T> typename multi_array<T>::shape_type multi_array<T>::shape() const noexcept
{
    return m_shape;
}

template<class T> std::size_t multi_array<T>::entries() const noexcept
{
    return m_data.size();
}

template<class T> std::pair<typename multi_array<T>::iterator, typename multi_array<T>::iterator> multi_array<T>::operator [](std::size_t index)
{
    if (m_shape.size() == 1)
    {
        if (index == 0)
        {
            return std::make_pair(m_data.begin(), m_data.end());
        }
    }
    else if (m_shape.size() == 2)
    {
        if (index == 0)
        {
            const std::size_t width = m_shape[0];
            const std::size_t height = m_shape[1];

            return std::make_pair(m_data.begin(), m_data.begin() + (width * height));
        }
    }
    else if (m_shape.size() == 3)
    {
        if (index < static_cast<std::size_t>(m_shape[2]))
        {
            const std::size_t width = m_shape[0];
            const std::size_t height = m_shape[1];
            const std::size_t entries = width * height;

            return std::make_pair(m_data.begin() + (entries * index), m_data.begin() + (entries * (index + 1)));
        }
    }

    throw std::out_of_range(std::string("index ").append(std::to_string(index)).append(" out of range"));
}

template<class T> std::pair<typename multi_array<T>::const_iterator, typename multi_array<T>::const_iterator> multi_array<T>::operator [](std::size_t index) const
{
    if (m_shape.size() == 1)
    {
        if (index == 0)
        {
            return std::make_pair(m_data.begin(), m_data.end());
        }
    }
    else if (m_shape.size() == 2)
    {
        if (index == 0)
        {
            const std::size_t width = m_shape[0];
            const std::size_t height = m_shape[1];

            return std::make_pair(m_data.begin(), m_data.begin() + (width * height));
        }
    }
    else if (m_shape.size() == 3)
    {
        if (index < static_cast<std::size_t>(m_shape[2]))
        {
            const std::size_t width = m_shape[0];
            const std::size_t height = m_shape[1];
            const std::size_t entries = width * height;

            return std::make_pair(m_data.begin() + (entries * index), m_data.begin() + (entries * (index + 1)));
        }
    }

    throw std::out_of_range(std::string("index ").append(std::to_string(index)).append(" out of range"));
}

template<class T> multi_array<T> & multi_array<T>::operator=(std::initializer_list<T> const & data)
{
    m_data = std::move(data);
    m_data.resize(m_capacity);

    return *this;
}

template<class T> multi_array<T> & multi_array<T>::operator=(std::vector<T> const & data)
{
    m_data = std::move(data);
    m_data.resize(m_capacity);

    return *this;
}

// manual instantation of multi_array<> for some types
template class multi_array<double>;
template class multi_array<float>;
template class multi_array<std::uint8_t>;

} // namespace cvpg
