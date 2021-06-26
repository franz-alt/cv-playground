// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_CORE_MULTI_ARRAY_HPP
#define LIBCVPG_CORE_MULTI_ARRAY_HPP

#include <cstdint>
#include <functional>
#include <initializer_list>
#include <numeric>
#include <type_traits>
#include <utility>
#include <vector>

namespace cvpg {

template<class T>
class multi_array
{
public:
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    using shape_type = std::vector<int>;

    using value_type = T;

    multi_array() = default;

    template<typename... Args, typename = typename std::enable_if_t<std::conjunction_v<std::is_same<int, Args>...> > >
    multi_array(Args... args)
        : m_shape({ std::forward<Args>(args)... })
    {
        m_capacity = std::accumulate(m_shape.cbegin(), m_shape.cend(), 1, std::multiplies<int>());
        m_data.resize(m_capacity);
    }

    multi_array(std::initializer_list<int> shape);

    multi_array(std::vector<int> shape);

    std::size_t dimensions() const noexcept;

    shape_type shape() const noexcept;

    std::size_t entries() const noexcept;

    std::pair<iterator, iterator> operator[](std::size_t index);

    std::pair<const_iterator, const_iterator> operator[](std::size_t index) const;

    // assign new data to the multi_array. data will be limited or filled-up to estimated size
    multi_array<T> & operator=(std::initializer_list<T> const & data);

    // assign new data to the multi_array. data will be limited or filled-up to estimated size
    multi_array<T> & operator=(std::vector<T> const & data);

private:
    shape_type m_shape;

    std::size_t m_capacity = 0;

    std::vector<T> m_data;
};

// suppress automatic instantiation of multi_array<> for some types
extern template class multi_array<double>;
extern template class multi_array<float>;
extern template class multi_array<std::uint8_t>;

} // namespace cvpg

#endif // LIBCVPG_CORE_MULTI_ARRAY_HPP
