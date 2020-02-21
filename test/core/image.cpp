#include <gtest/gtest.h>

#include <libcvpg/core/image.hpp>

TEST(test_image, image_creation)
{
    // default ctor has to create an empty grayscale image
    {
        auto image = cvpg::image_gray_8bit();
        ASSERT_TRUE(image.width() == 0 && image.height() == 0);
    }

    // create a grayscale image of size 1920x1080 pixels
    {
        auto image = cvpg::image_gray_8bit(1920, 1080);
        ASSERT_TRUE(image.width() == 1920 && image.height() == 1080);
    }
}
