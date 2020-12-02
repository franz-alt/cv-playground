#include <libcvpg/core/metadata.hpp>

namespace cvpg {

void metadata::push(key_type const & key, value_type const & value)
{
    m_data.insert({ key, value });
}

void metadata::emplace(key_type && key, value_type && value)
{
    m_data.emplace(std::forward<key_type>(key), std::forward<value_type>(value));
}

std::vector<metadata::key_type> metadata::keys() const
{
    std::vector<key_type> keys;
    keys.reserve(m_data.size());

    for (auto const & [key, value] : m_data)
    {
        keys.push_back(key);
    }

    return keys;
}

std::size_t metadata::size() const noexcept
{
    return m_data.size();
}

metadata::iterator_type metadata::begin() noexcept
{
    return m_data.begin();
}

metadata::iterator_type metadata::end() noexcept
{
    return m_data.end();
}

metadata::const_iterator_type metadata::cbegin() const noexcept
{
    return m_data.cbegin();
}

metadata::const_iterator_type metadata::cend() const noexcept
{
    return m_data.cend();
}

bool metadata::contains(key_type const & key) const
{
    auto it = m_data.find(key);

    return it != m_data.cend();
}

metadata::iterator_type metadata::find(key_type const & key)
{
    return m_data.find(key);
}

metadata::const_iterator_type metadata::find(key_type const & key) const
{
    return m_data.find(key);
}

} // namespace cvpg
