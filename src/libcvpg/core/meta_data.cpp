// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/core/meta_data.hpp>

namespace cvpg {

void meta_data::push(key_type const & key, value_type const & value)
{
    m_data.insert({ key, value });
}

void meta_data::emplace(key_type && key, value_type && value)
{
    m_data.emplace(std::forward<key_type>(key), std::forward<value_type>(value));
}

std::vector<meta_data::key_type> meta_data::keys() const
{
    std::vector<key_type> keys;
    keys.reserve(m_data.size());

    for (auto const & [key, value] : m_data)
    {
        keys.push_back(key);
    }

    return keys;
}

std::size_t meta_data::size() const noexcept
{
    return m_data.size();
}

meta_data::iterator_type meta_data::begin() noexcept
{
    return m_data.begin();
}

meta_data::iterator_type meta_data::end() noexcept
{
    return m_data.end();
}

meta_data::const_iterator_type meta_data::cbegin() const noexcept
{
    return m_data.cbegin();
}

meta_data::const_iterator_type meta_data::cend() const noexcept
{
    return m_data.cend();
}

bool meta_data::contains(key_type const & key) const
{
    auto it = m_data.find(key);

    return it != m_data.cend();
}

meta_data::iterator_type meta_data::find(key_type const & key)
{
    return m_data.find(key);
}

meta_data::const_iterator_type meta_data::find(key_type const & key) const
{
    return m_data.find(key);
}

} // namespace cvpg
