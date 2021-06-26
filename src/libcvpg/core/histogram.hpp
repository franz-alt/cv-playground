// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_CORE_HISTOGRAM_HPP
#define LIBCVPG_CORE_HISTOGRAM_HPP

#include <iterator>
#include <ostream>
#include <vector>

namespace cvpg {

template<class value = std::size_t>
class histogram
{
public:
    using value_type = value;
    using reference_type = typename std::vector<value_type>::reference;
    using iterator_type = typename std::vector<value_type>::iterator;
    using const_reference_type = typename std::vector<value_type>::const_reference;
    using const_iterator_type = typename std::vector<value_type>::const_iterator;

    // create a histogram with 256 bins
    histogram();
    histogram(std::size_t bins);
    histogram(std::vector<value_type> data);

    histogram(histogram const &) = default;
    histogram(histogram &&) = default;

    histogram & operator=(histogram const &) = default;
    histogram & operator=(histogram &&) = default;

    histogram & operator=(std::vector<value_type> data);

    std::size_t bins() const noexcept;

    iterator_type begin() noexcept;
    iterator_type end() noexcept;

    const_iterator_type cbegin() const noexcept;
    const_iterator_type cend() const noexcept;

    reference_type at(std::size_t i);
    const_reference_type at(std::size_t i) const;

    histogram operator+(histogram const & rhs) const;
    histogram & operator+=(histogram const & rhs);

private:
    std::vector<value_type> m_data;
};

// suppress automatic instantiation of histogram<> for some types
extern template class histogram<std::size_t>;
extern template class histogram<double>;

template<class value_type = std::size_t>
typename histogram<value_type>::const_iterator_type begin(histogram<value_type> const & h) noexcept;

template<class value_type = std::size_t>
typename histogram<value_type>::const_iterator_type end(histogram<value_type> const & h) noexcept;

template<class value_type = std::size_t>
std::ostream & operator<<(std::ostream & out, histogram<value_type> const & h);

} // namespace cvpg

#endif // LIBCVPG_CORE_HISTOGRAM_HPP
