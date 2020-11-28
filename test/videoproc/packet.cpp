#include <gtest/gtest.h>

#include <libcvpg/core/image.hpp>
#include <libcvpg/videoproc/frame.hpp>
#include <libcvpg/videoproc/packet.hpp>

TEST(test_packet, default_packet)
{
    // default ctor has to create an empty packet with number 0
    cvpg::videoproc::packet<cvpg::videoproc::frame<cvpg::image_gray_8bit> > packet;   

    ASSERT_EQ(packet.number(), 0);
}

TEST(test_packet, flush_packet)
{
    // create a flush packet with a specified number
    cvpg::videoproc::packet<cvpg::videoproc::frame<cvpg::image_gray_8bit> > packet(42);
    packet.add_frame(cvpg::videoproc::frame<cvpg::image_gray_8bit>(11));

    ASSERT_EQ(packet.number(), 42);
    ASSERT_EQ(packet.flush(), true);
}

TEST(test_packet, add_frames)
{
    // create a normal packet with a specified number
    cvpg::videoproc::packet<cvpg::videoproc::frame<cvpg::image_gray_8bit> > packet(1);

    // create some test frames
    for (std::size_t i = 0; i < 10; ++i)
    {
        cvpg::image_gray_8bit image(1920, 1080);

        packet.add_frame(cvpg::videoproc::frame<cvpg::image_gray_8bit>(i, std::move(image)));
    }

    ASSERT_EQ(packet.frames().size(), 10);
    ASSERT_EQ(packet.flush(), false);

    // add a flush frame at the end
    packet.add_frame(cvpg::videoproc::frame<cvpg::image_gray_8bit>(11));

    ASSERT_EQ(packet.frames().size(), 11);
    ASSERT_EQ(packet.flush(), true);
}

TEST(test_packet, move_frames)
{
    // create a normal packet with a specified number
    cvpg::videoproc::packet<cvpg::videoproc::frame<cvpg::image_gray_8bit> > packet(1);

    // create some test frames
    for (std::size_t i = 0; i < 10; ++i)
    {
        cvpg::image_gray_8bit image(1920, 1080);

        packet.add_frame(cvpg::videoproc::frame<cvpg::image_gray_8bit>(i, std::move(image)));
    }

    // move frames out of the packet
    auto frames = packet.move_frames();

    ASSERT_EQ(packet.frames().size(), 0);
}
