#ifndef LIBCVPG_CORE_HISTOGRAM_HPP
#define LIBCVPG_CORE_HISTOGRAM_HPP

#include <iterator>
#include <ostream>
#include <vector>

namespace cvpg {

class histogram
{
public:
    using value_type = std::size_t;
    using reference_type = std::vector<value_type>::reference;
    using iterator_type = std::vector<value_type>::iterator;
    using const_reference_type = std::vector<value_type>::const_reference;
    using const_iterator_type = std::vector<value_type>::const_iterator;

    // create a histogram with 256 bins
    histogram();
    histogram(std::size_t bins);
    histogram(std::vector<value_type> data);

    histogram(histogram const &) = default;
    histogram(histogram &&) = default;

    histogram & operator=(histogram const &) = default;
    histogram & operator=(histogram &&) = default;

    histogram & operator=(std::vector<value_type> data);

    std::size_t bins() const;

    iterator_type begin() noexcept;
    iterator_type end() noexcept;

    const_iterator_type cbegin() const noexcept;
    const_iterator_type cend() const noexcept;

    reference_type at(std::size_t i);
    const_reference_type at(std::size_t i) const;

    histogram operator+(histogram const & rhs) const;
    histogram & operator+=(histogram const & rhs);

private:
    std::vector<std::size_t> m_data;
};

histogram::const_iterator_type begin(histogram const & h) noexcept;
histogram::const_iterator_type end(histogram const & h) noexcept;

std::ostream & operator<<(std::ostream & out, histogram const & h);

} // namespace cvpg

#endif // LIBCVPG_CORE_HISTOGRAM_HPP
