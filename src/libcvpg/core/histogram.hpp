#ifndef LIBCVPG_CORE_HISTOGRAM_HPP
#define LIBCVPG_CORE_HISTOGRAM_HPP

#include <iterator>
#include <ostream>
#include <vector>

namespace cvpg {

class histogram
{
public:
    using iterator_type = std::vector<std::size_t>::const_iterator;

    histogram();
    
    histogram(histogram const &) = default;
    histogram(histogram &&) = default;

    histogram(std::vector<std::size_t> data);

    histogram & operator=(histogram const &) = default;
    histogram & operator=(histogram &&) = default;

    histogram & operator=(std::vector<std::size_t> data);

    std::size_t bins() const;

    iterator_type begin() const;
    iterator_type end() const;

    std::size_t at(std::size_t i) const;

    histogram operator+(histogram const & rhs) const;
    histogram & operator+=(histogram const & rhs);

private:
    std::vector<std::size_t> m_data;
};

histogram::iterator_type begin(histogram const & h);
histogram::iterator_type end(histogram const & h);

std::ostream & operator<<(std::ostream & out, histogram const & h);

} // namespace cvpg

#endif // LIBCVPG_CORE_HISTOGRAM_HPP
