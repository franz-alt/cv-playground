#include <gtest/gtest.h>

#include <libcvpg/core/histogram.hpp>

TEST(test_histogram, default_histogram)
{
    // default ctor has to create an empty histogram with 256 bins
    cvpg::histogram<std::size_t> h;

    ASSERT_EQ(h.bins(), 256);
}

TEST(test_histogram, custom_size)
{
    const std::size_t bins = 11;

    // create a histogram with a certain amount of bins
    cvpg::histogram<std::size_t> h(bins);

    ASSERT_EQ(h.bins(), bins);
}

TEST(test_histogram, access_valid_index)
{
    // create histogram with 256 bins
    cvpg::histogram<std::size_t> h;

    try
    {
        ASSERT_EQ(h.at(42), 0);
    }
    catch (...)
    {
        ASSERT_TRUE(false);
    }
}

TEST(test_histogram, access_invalid_index)
{
    // create histogram with 256 bins
    {
        cvpg::histogram<std::size_t> h;

        try
        {
            ASSERT_EQ(h.at(-5), 0);
            ASSERT_TRUE(false);
        }
        catch (...)
        {
            // ok
        }

        try
        {
            ASSERT_EQ(h.at(1000), 0);
            ASSERT_TRUE(false);
        }
        catch (...)
        {
            // ok
        }
    }

    // create histogram with 42 bins
    {
        const std::size_t bins = 42;

        // create a histogram with a certain amount of bins
        cvpg::histogram<std::size_t> h(bins);

        try
        {
            ASSERT_EQ(h.at(bins + 1), 0);
            ASSERT_TRUE(false);
        }
        catch (...)
        {
            // ok
        }
    }
}

TEST(test_histogram, modify_bin_values)
{
    // create histogram with 256 bins
    {
        cvpg::histogram<std::size_t> h;

        ASSERT_EQ(h.at(5), 0);

        h.at(5)++;

        ASSERT_EQ(h.at(5), 1);

        h.at(5) += 42;

        ASSERT_EQ(h.at(5), 43);

        h.at(5) = 100000;

        ASSERT_EQ(h.at(5), 100000);
    }
}

TEST(test_histogram, add_histograms)
{
    // create histograms both with 256 bins
    cvpg::histogram<std::size_t> h1;
    cvpg::histogram<std::size_t> h2;

    // fill some values
    h1.at(42) = 5;
    h2.at(42) = 10;
    h2.at(43) = 7;

    // add histograms to a new one
    cvpg::histogram<std::size_t> h3 = h1 + h2;

    ASSERT_EQ(h3.bins(), 256);
    ASSERT_EQ(h3.at(0), 0);
    ASSERT_EQ(h3.at(42), 15);
    ASSERT_EQ(h3.at(43), 7);
}

TEST(test_histogram, add_histograms_different_size)
{
    // create histograms with different bin sizes
    cvpg::histogram<std::size_t> h1;
    cvpg::histogram<std::size_t> h2(42);

    // try to add histograms
    try
    {
        cvpg::histogram<std::size_t> h3 = h1 + h2;
        ASSERT_TRUE(false);
    }
    catch (...)
    {
        // ok
    }
}
