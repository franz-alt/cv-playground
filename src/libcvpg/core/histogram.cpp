// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/core/histogram.hpp>

#include <algorithm>
#include <stdexcept>

namespace cvpg {

template<class value> histogram<value>::histogram()
    : histogram(256)
{}

template<class value> histogram<value>::histogram(std::size_t bins)
    : m_data(bins)
{}

template<class value> histogram<value>::histogram(std::vector<value_type> data)
    : m_data(std::move(data))
{}

template<class value> histogram<value> & histogram<value>::operator=(std::vector<value_type> data)
{
    m_data = std::move(data);

    return *this;
}

template<class value> std::size_t histogram<value>::bins() const noexcept
{
    return m_data.size();
}

template<class value> typename histogram<value>::iterator_type histogram<value>::begin() noexcept
{
    return m_data.begin();
}

template<class value> typename histogram<value>::iterator_type histogram<value>::end() noexcept
{
    return m_data.end();
}

template<class value> typename histogram<value>::const_iterator_type histogram<value>::cbegin() const noexcept
{
    return m_data.cbegin();
}

template<class value> typename histogram<value>::const_iterator_type histogram<value>::cend() const noexcept
{
    return m_data.cend();
}

template<class value> typename histogram<value>::reference_type histogram<value>::at(std::size_t i)
{
    return m_data.at(i);
}

template<class value> typename histogram<value>::const_reference_type histogram<value>::at(std::size_t i) const
{
    return m_data.at(i);
}

template<class value> histogram<value> histogram<value>::operator+(histogram const & rhs) const
{
    if (bins() != rhs.bins())
    {
        throw std::runtime_error("different bin sizes");
    }

    std::vector<value_type> h(m_data.size());

    std::transform(m_data.begin(),
                   m_data.end(),
                   rhs.m_data.begin(),
                   h.begin(),
                   [](std::size_t a, std::size_t b)
                   {
                       return a + b;
                   });

    return histogram(std::move(h));
}

template<class value> histogram<value> & histogram<value>::operator+=(histogram const & rhs)
{
    if (bins() != rhs.bins())
    {
        throw std::runtime_error("different bin sizes");
    }

    std::vector<value_type> h(m_data.size());

    std::transform(m_data.begin(),
                   m_data.end(),
                   rhs.m_data.begin(),
                   h.begin(),
                   [](std::size_t a, std::size_t b)
                   {
                       return a + b;
                   });

    std::swap(h, m_data);

    return *this;
}

// manual instantation of histogram<> for some types
template class histogram<std::size_t>;
template class histogram<double>;

template<class value>
typename histogram<value>::const_iterator_type begin(histogram<value> const & h) noexcept
{
    return h.cbegin();
}

template<class value>
typename histogram<value>::const_iterator_type end(histogram<value> const & h) noexcept
{
    return h.cend();
}

template<class value>
std::ostream & operator<<(std::ostream & out, histogram<value> const & h)
{
    out << "bins=" << h.bins();

    return out;
}

} // namespace cvpg
