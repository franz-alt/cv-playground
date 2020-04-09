#include <libcvpg/core/histogram.hpp>

#include <algorithm>

namespace cvpg {

histogram::histogram()
    : m_data(256)
{}

histogram::histogram(std::vector<std::size_t> data)
    : m_data(std::move(data))
{}

histogram & histogram::operator=(std::vector<std::size_t> data)
{
    m_data = std::move(data);

    return *this;
}

std::size_t histogram::bins() const
{
    return m_data.size();
}

std::vector<std::size_t>::const_iterator histogram::begin() const
{
    return m_data.begin();
}

std::vector<std::size_t>::const_iterator histogram::end() const
{
    return m_data.end();
}

std::size_t histogram::at(std::size_t i) const
{
    return m_data.at(i);
}

histogram histogram::operator+(histogram const & rhs) const
{
    // TODO error handling

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
    // TODO error handling

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

histogram::iterator_type begin(histogram const & h)
{
    return h.begin();
}

histogram::iterator_type end(histogram const & h)
{
    return h.end();
}

std::ostream & operator<<(std::ostream & out, histogram const & h)
{
    out << "bins=" << h.bins();

    return out;
}

} // namespace cvpg
