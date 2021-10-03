#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>

#include <libcvpg/core/multi_array.hpp>

TEST(test_multi_array, default_creation)
{
    cvpg::multi_array<std::uint8_t> array;

    // default created multi array means empty object means 0 dimensions
    ASSERT_EQ(array.dimensions(), 0);
}

TEST(test_multi_array, create_two_dimensional)
{
    cvpg::multi_array<std::uint8_t> array(4, 8);

    ASSERT_EQ(array.dimensions(), 2);

    auto shape = array.shape();

    ASSERT_EQ(shape.size(), 2);

    ASSERT_EQ(shape[0], 4);
    ASSERT_EQ(shape[1], 8);

    ASSERT_EQ(array.entries(), 4 * 8);
}

TEST(test_multi_array, create_dims_from_vector)
{
    cvpg::multi_array<std::uint8_t> array(std::vector<int>({ 2, 9 }));

    ASSERT_EQ(array.dimensions(), 2);
    ASSERT_EQ(array.entries(), 2 * 9);
}

TEST(test_multi_array, create_dims_from_initializer_list)
{
    cvpg::multi_array<std::uint8_t> array({ 2, 9 });

    ASSERT_EQ(array.dimensions(), 2);
    ASSERT_EQ(array.entries(), 2 * 9);
}

TEST(test_multi_array, fill_with_data)
{
    // case 1: fill a one dimensional array with data of suitable size
    {
        cvpg::multi_array<std::uint8_t> array(3);

        array = { 1, 2, 3 };

        ASSERT_EQ(array.entries(), 3);
    }

    // case 2: fill a one dimensional array with data of less than expected size
    {
        cvpg::multi_array<std::uint8_t> array(3);

        array = { 42 };

        ASSERT_EQ(array.entries(), 3);
    }

    // case 3: fill a one dimensional array with more data then expected
    {
        cvpg::multi_array<std::uint8_t> array(3);

        array = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

        ASSERT_EQ(array.entries(), 3);
    }
}

TEST(test_multi_array, one_dimension_access_data)
{
    cvpg::multi_array<std::uint8_t> array(2);

    ASSERT_EQ(array.dimensions(), 1);
    ASSERT_EQ(array.entries(), 2);

    // fill array with some data
    array = { 42, 5 };

    auto [begin, end] = array[0];

    ASSERT_EQ(std::distance(begin, end), 2);

    // check data on iterator level
    ASSERT_EQ(*(begin++), 42);
    ASSERT_EQ(*begin, 5);
}

TEST(test_multi_array, two_dimensions_access_data)
{
    cvpg::multi_array<std::uint8_t> array(2, 4);

    ASSERT_EQ(array.dimensions(), 2);
    ASSERT_EQ(array.entries(), 2 * 4);

    // fill array with some data
    array = {
                1, 2,
                3, 4,
                5, 6,
                7, 8
            };

    // check first range
    {
        auto [begin, end] = array[0];

        ASSERT_EQ(std::distance(begin, end), 8);

        // check data on iterator level
        ASSERT_EQ(*(begin++), 1);
        ASSERT_EQ(*(begin++), 2);
        ASSERT_EQ(*(begin++), 3);
        ASSERT_EQ(*(begin++), 4);
        ASSERT_EQ(*(begin++), 5);
        ASSERT_EQ(*(begin++), 6);
        ASSERT_EQ(*(begin++), 7);
        ASSERT_EQ(*begin, 8);
    }

    // check second range (not available)
    {
        ASSERT_THROW(array[1], std::out_of_range);
    }
}

TEST(test_multi_array, three_dimensions_access_data)
{
    cvpg::multi_array<std::uint8_t> array(3, 2, 4);

    ASSERT_EQ(array.dimensions(), 3);
    ASSERT_EQ(array.entries(), 3 * 2 * 4);

    // fill array with some data
    array = {
                // first dimension
                1, 1, 1, 1, 1, 1,
                // second dimension
                2, 2, 2, 2, 2, 2,
                // third dimension
                3, 3, 3, 3, 3, 3,
                // fourth dimension
                4, 4, 4, 4, 4, 4
            };

    // check first dimension
    {
        auto [begin, end] = array[0];

        ASSERT_EQ(std::distance(begin, end), 6);

        // check data on iterator level
        std::for_each(begin,
                      end,
                      [](auto const & v)
                      {
                          ASSERT_EQ(v, 1);
                      });

        // check const-version
        auto const & const_array = array;

        auto [cbegin, cend] = const_array[0];

        ASSERT_EQ(std::distance(cbegin, cend), 6);
    }

    // check second dimension
    {
        auto [begin, end] = array[1];

        ASSERT_EQ(std::distance(begin, end), 6);

        // check data on iterator level
        std::for_each(begin,
                      end,
                      [](auto const & v)
                      {
                          ASSERT_EQ(v, 2);
                      });
    }

    // check third dimension
    {
        auto [begin, end] = array[2];

        ASSERT_EQ(std::distance(begin, end), 6);

        // check data on iterator level
        std::for_each(begin,
                      end,
                      [](auto const & v)
                      {
                          ASSERT_EQ(v, 3);
                      });
    }

    // check fourth dimension
    {
        auto [begin, end] = array[3];

        ASSERT_EQ(std::distance(begin, end), 6);

        // check data on iterator level
        std::for_each(begin,
                      end,
                      [](auto const & v)
                      {
                          ASSERT_EQ(v, 4);
                      });
    }

    // check fifth dimension (not available)
    {
        ASSERT_THROW(array[4], std::out_of_range);
    }
}

TEST(test_multi_array, copy_array)
{
    cvpg::multi_array<std::uint8_t> array(2, 3);

    ASSERT_EQ(array.dimensions(), 2);
    ASSERT_EQ(array.entries(), 2 * 3);

    // fill array with some data
    array = { 42, 42, 42, 42, 42, 42 };

    // create a copy
    auto array2 = array;

    ASSERT_EQ(array2.dimensions(), 2);
    ASSERT_EQ(array2.entries(), 2 * 3);

    // check first dimension of copy
    {
        auto [begin, end] = array2[0];

        ASSERT_EQ(std::distance(begin, end), 6);

        // check data on iterator level
        std::for_each(begin,
                      end,
                      [](auto const & v)
                      {
                          ASSERT_EQ(v, 42);
                      });
    }
}
