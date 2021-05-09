#include <gtest/gtest.h>

#include <libcvpg/core/meta_data.hpp>

TEST(test_metadata, default_creation)
{
    // create an empty metadata object
    cvpg::meta_data data;

    // no entries
    ASSERT_EQ(data.size(), 0);

    // no key named 'foo'
    ASSERT_FALSE(data.contains("foo"));

    // no key 'bar' found
    ASSERT_TRUE(data.find("bar") == data.cend());
}

TEST(test_metadata, filled_data)
{
    cvpg::meta_data data;

    // add new entry with key 'foo' and value 42, converted to a std::any
    data.emplace("foo", 42);

    ASSERT_EQ(data.size(), 1);

    ASSERT_TRUE(data.contains("foo"));

    auto it = data.find("foo");

    ASSERT_TRUE(it != data.cend());

    // extract int from std::any and compare
    ASSERT_EQ(std::any_cast<int>(it->second), 42);
}

TEST(test_metadata, keys)
{
    // create an empty metadata object
    cvpg::meta_data data;

    // no keys
    ASSERT_TRUE(data.keys().empty());

    // fill with some data
    data.emplace("foo", 42);
    data.push(std::string("bar"), "123");
    data.emplace(std::string("baz"), 3.1415);

    // three keys
    const auto keys = data.keys();

    ASSERT_EQ(keys.size(), 3);

    ASSERT_TRUE(std::find_if(keys.begin(), keys.end(), [](auto const & key){ return key == "foo"; }) != keys.end());
    ASSERT_TRUE(std::find_if(keys.begin(), keys.end(), [](auto const & key){ return key == "bar"; }) != keys.end());
    ASSERT_TRUE(std::find_if(keys.begin(), keys.end(), [](auto const & key){ return key == "baz"; }) != keys.end());
}
