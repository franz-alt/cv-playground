#include <gtest/gtest.h>

#include <libcvpg/core/image.hpp>
#include <libcvpg/videoproc/frame.hpp>
#include <libcvpg/videoproc/stage_data_handler.hpp>

TEST(test_stage_data_handler, add_frames_correct_order)
{
    using frame = cvpg::videoproc::frame<cvpg::image_gray_8bit>;

    std::size_t deliver_counter = 0;

    cvpg::videoproc::stage_data_handler<frame> sdh(
        "test",
        5,
        []()
        {
            return true;
        },
        []()
        {
            return true;
        },
        [&deliver_counter](auto frames, std::function<void()> deliver_done_callback)
        {
            ++deliver_counter;
        },
        []()
        {
            ASSERT_TRUE(false);
        }
    );

    // add 3 frames ; after this the data should be delivered to next stage
    sdh.add(frame(0));
    sdh.add(frame(1));
    sdh.add(frame(2));

    ASSERT_EQ(deliver_counter, 3);
}

TEST(test_stage_data_handler, add_frames_random_order)
{
    using frame = cvpg::videoproc::frame<cvpg::image_gray_8bit>;

    std::size_t deliver_counter = 0;

    cvpg::videoproc::stage_data_handler<frame> sdh(
        "test",
        5,
        []()
        {
            return true;
        },
        []()
        {
            return 3;
        },
        [&deliver_counter](auto frames, std::function<void()> deliver_done_callback)
        {
            ++deliver_counter;
        },
        []()
        {
            ASSERT_TRUE(false);
        }
    );

    sdh.add(frame(3));
    sdh.add(frame(4));
    sdh.add(frame(2));
    sdh.add(frame(0));
    sdh.add(frame(1));
    sdh.add(frame(5));
    sdh.add(frame(7));
    sdh.add(frame(9));
    sdh.add(frame(8));
    sdh.add(frame(6));

    ASSERT_EQ(deliver_counter, 4);
}

TEST(test_stage_data_handler, frame_missing)
{
    using frame = cvpg::videoproc::frame<cvpg::image_gray_8bit>;

    std::size_t frames_delivered = 0;

    cvpg::videoproc::stage_data_handler<frame> sdh(
        "test",
        5,
        []()
        {
            return true;
        },
        []()
        {
            return true;
        },
        [&frames_delivered](auto frames, std::function<void()> deliver_done_callback)
        {
            frames_delivered += frames.size();
        },
        []()
        {
            ASSERT_TRUE(true);
        }
    );

    sdh.add(frame(0));
    sdh.add(frame(1));
    sdh.add(frame(2));

    // frame #3 is missing -> buffer full callback has to be called

    sdh.add(frame(4));
    sdh.add(frame(5));
    sdh.add(frame(6));
    sdh.add(frame(7));
    sdh.add(frame(8));
    sdh.add(frame(9));

    ASSERT_EQ(frames_delivered, 3);
}

TEST(test_stage_data_handler, try_flush)
{
    using frame = cvpg::videoproc::frame<cvpg::image_gray_8bit>;

    std::size_t deliver_counter = 0;

    cvpg::videoproc::stage_data_handler<frame> sdh(
        "test",
        5,
        []()
        {
            return true;
        },
        []()
        {
            return 5;
        },
        [&deliver_counter](auto frames, std::function<void()> deliver_done_callback)
        {
            ++deliver_counter;
        },
        []()
        {
            ASSERT_TRUE(true);
        }
    );

    sdh.add(frame(0));
    sdh.add(frame(1));

    sdh.try_flush();

    sdh.add(frame(2));

    ASSERT_EQ(deliver_counter, 3);
}

TEST(test_stage_data_handler, fill_many_entries)
{
    using frame = cvpg::videoproc::frame<cvpg::image_gray_8bit>;

    std::size_t deliver_counter = 0;

    cvpg::videoproc::stage_data_handler<frame> sdh(
        "test",
        5,
        []()
        {
            return true;
        },
        []()
        {
            return 3;
        },
        [&deliver_counter](auto frames, std::function<void()> deliver_done_callback)
        {
            ++deliver_counter;
        },
        []()
        {
            ASSERT_TRUE(true);
        }
    );

    const std::size_t entries = 10000;

    for (std::size_t i = 0; i < entries; ++i)
    {
        sdh.add(frame(i));
    }

    ASSERT_EQ(deliver_counter, entries);
}
