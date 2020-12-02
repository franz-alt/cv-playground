#ifndef LIBCVPG_CORE_METADATA_HPP
#define LIBCVPG_CORE_METADATA_HPP

#include <any>
#include <map>
#include <string>
#include <vector>

namespace cvpg {

class metadata
{
public:
    using key_type = std::string;
    using value_type = std::any;
    using container_type = std::map<key_type, value_type>;
    using reference_type = container_type::reference;
    using iterator_type = container_type::iterator;
    using const_reference_type = container_type::const_reference;
    using const_iterator_type = container_type::const_iterator;

    void push(key_type const & key, value_type const & value);

    void emplace(key_type && key, value_type && value);

    std::vector<key_type> keys() const;

    std::size_t size() const noexcept;

    iterator_type begin() noexcept;
    iterator_type end() noexcept;

    const_iterator_type cbegin() const noexcept;
    const_iterator_type cend() const noexcept;

    bool contains(key_type const & key) const;

    iterator_type find(key_type const & key);
    const_iterator_type find(key_type const & key) const;

private:
    container_type m_data;
};

} // namespace cvpg

#endif // LIBCVPG_CORE_METADATA_HPP
