#include <libcvpg/core/histogram.hpp>

#include <algorithm>
#include <stdexcept>

namespace cvpg {

histogram::histogram()
    : histogram(256)
{}

histogram::histogram(std::size_t bins)
    : m_data(bins)
{}

histogram::histogram(std::vector<value_type> data)
    : m_data(std::move(data))
{}

histogram & histogram::operator=(std::vector<std::size_t> data)
{
    m_data = std::move(data);

    return *this;
}

std::size_t histogram::bins() const noexcept
{
    return m_data.size();
}

histogram::iterator_type histogram::begin() noexcept
{
    return m_data.begin();
}

histogram::iterator_type histogram::end() noexcept
{
    return m_data.end();
}

histogram::const_iterator_type histogram::cbegin() const noexcept
{
    return m_data.cbegin();
}

histogram::const_iterator_type histogram::cend() const noexcept
{
    return m_data.cend();
}

histogram::reference_type histogram::at(std::size_t i)
{
    return m_data.at(i);
}

histogram::const_reference_type histogram::at(std::size_t i) const
{
    return m_data.at(i);
}

histogram histogram::operator+(histogram const & rhs) const
{
    if (bins() != rhs.bins())
    {
        throw std::runtime_error("different bin sizes");
    }

    std::vector<std::size_t> h(m_data.size());

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

histogram & histogram::operator+=(histogram const & rhs)
{
    if (bins() != rhs.bins())
    {
        throw std::runtime_error("different bin sizes");
    }

    std::vector<std::size_t> h(m_data.size());

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

histogram::const_iterator_type begin(histogram const & h) noexcept
{
    return h.cbegin();
}

histogram::const_iterator_type end(histogram const & h) noexcept
{
    return h.cend();
}

std::ostream & operator<<(std::ostream & out, histogram const & h)
{
    out << "bins=" << h.bins();

    return out;
}

} // namespace cvpg
