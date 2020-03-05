#include <gtest/gtest.h>

#include <string>
#include <thread>

#include <boost/asynchronous/queue/lockfree_queue.hpp>
#include <boost/asynchronous/scheduler_shared_proxy.hpp>
#include <boost/asynchronous/scheduler/multiqueue_threadpool_scheduler.hpp>
#include <boost/asynchronous/scheduler/single_thread_scheduler.hpp>

#include <libcvpg/imageproc/scripting/image_processor.hpp>

TEST(test_scripting, compile_invalid_script)
{
    // create a thread pool for a single thread
    auto pool = boost::asynchronous::make_shared_scheduler_proxy<
                    boost::asynchronous::multiqueue_threadpool_scheduler<
                        boost::asynchronous::lockfree_queue<> > >(1, std::string("threadpool"));

    // create image processor
    auto scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                        boost::asynchronous::single_thread_scheduler<
                            boost::asynchronous::lockfree_queue<> > >(std::string("image_processor"));

    cvpg::imageproc::scripting::image_processor_proxy image_processor(scheduler, pool);

    // case: 'val' instead of 'var'
    {
        auto promise_compile = std::make_shared<std::promise<std::size_t> >();
        auto future_compile = promise_compile->get_future();

        image_processor.compile(
            R"(val input_rgb = input("rgb", 8))",
            [promise_compile](bool successful, std::size_t compile_id)
            {
                ASSERT_TRUE(!successful);

                promise_compile->set_value(compile_id);
            }
        );

        auto status = future_compile.wait_for(std::chrono::seconds(3));

        ASSERT_TRUE(status != std::future_status::deferred && status != std::future_status::timeout);
    }

    // case: unknown algoritm
    {
        auto promise_compile = std::make_shared<std::promise<std::size_t> >();
        auto future_compile = promise_compile->get_future();

        image_processor.compile(
            R"(var input_rgb = input_("rgb", 8))",
            [promise_compile](bool successful, std::size_t compile_id)
            {
                ASSERT_TRUE(!successful);

                promise_compile->set_value(compile_id);
            }
        );

        auto status = future_compile.wait_for(std::chrono::seconds(3));

        ASSERT_TRUE(status != std::future_status::deferred && status != std::future_status::timeout);
    }
}

TEST(test_scripting, compile_simple_script)
{
    // create a thread pool for a single thread
    auto pool = boost::asynchronous::make_shared_scheduler_proxy<
                    boost::asynchronous::multiqueue_threadpool_scheduler<
                        boost::asynchronous::lockfree_queue<> > >(1, std::string("threadpool"));

    // create image processor
    auto scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                        boost::asynchronous::single_thread_scheduler<
                            boost::asynchronous::lockfree_queue<> > >(std::string("image_processor"));

    cvpg::imageproc::scripting::image_processor_proxy image_processor(scheduler, pool);

    // first compile
    {
        auto promise_compile = std::make_shared<std::promise<std::size_t> >();
        auto future_compile = promise_compile->get_future();

        image_processor.compile(
            R"(
                var input_rgb = input("rgb", 8)
                var input_gray = convert_to_gray(input_rgb, "use_red")
            )",
            [promise_compile](bool successful, std::size_t compile_id)
            {
                ASSERT_TRUE(successful);

                promise_compile->set_value(compile_id);
            }
        );

        auto status = future_compile.wait_for(std::chrono::seconds(3));

        ASSERT_TRUE(status != std::future_status::deferred && status != std::future_status::timeout);
    }

    // second compile
    {
        auto promise_compile = std::make_shared<std::promise<std::size_t> >();
        auto future_compile = promise_compile->get_future();

        image_processor.compile(
            R"(
                var input_rgb = input("rgb", 8)
                var input_gray = convert_to_gray(input_rgb, "use_red")
            )",
            [promise_compile](bool successful, std::size_t compile_id)
            {
                ASSERT_TRUE(successful);

                promise_compile->set_value(compile_id);
            }
        );

        auto status = future_compile.wait_for(std::chrono::seconds(3));

        ASSERT_TRUE(status != std::future_status::deferred && status != std::future_status::timeout);
    }
}

TEST(test_scripting, evaluate_simple_script)
{
    // create a thread pool for a single thread
    auto pool = boost::asynchronous::make_shared_scheduler_proxy<
                    boost::asynchronous::multiqueue_threadpool_scheduler<
                        boost::asynchronous::lockfree_queue<> > >(1, std::string("threadpool"));

    // create image processor
    auto scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                        boost::asynchronous::single_thread_scheduler<
                            boost::asynchronous::lockfree_queue<> > >(std::string("scope"));

    cvpg::imageproc::scripting::image_processor_proxy image_processor(scheduler, pool);

    std::size_t compile_id = 0;

    // compile expression
    {
        auto promise_compile = std::make_shared<std::promise<std::size_t> >();
        auto future_compile = promise_compile->get_future();

        image_processor.compile(
            R"(
                var input_rgb = input("rgb", 8)
                var input_gray = convert_to_gray(input_rgb, "use_red")
            )",
            [promise_compile](bool successful, std::size_t compile_id)
            {
                ASSERT_TRUE(successful);

                promise_compile->set_value(compile_id);
            }
        );

        auto status = future_compile.wait_for(std::chrono::seconds(3));

        ASSERT_TRUE(status != std::future_status::deferred && status != std::future_status::timeout);

        compile_id = future_compile.get();
    }

    // evaluate image
    {
        cvpg::image_rgb_8bit image(1920, 1080);

        auto promise_evaluate = std::make_shared<std::promise<cvpg::image_gray_8bit> >();
        auto future_evaluate = promise_evaluate->get_future();

        image_processor.evaluate(
            compile_id,
            std::move(image),
            [promise_evaluate](cvpg::imageproc::scripting::item item)
            {
                ASSERT_TRUE(item.type() == cvpg::imageproc::scripting::item::types::grayscale_8_bit_image);

                auto image = std::any_cast<cvpg::image_gray_8bit>(item.value());

                promise_evaluate->set_value(std::move(image));
            }
        );

        auto status = future_evaluate.wait_for(std::chrono::seconds(3));

        ASSERT_TRUE(status != std::future_status::deferred && status != std::future_status::timeout);

        auto converted_image = future_evaluate.get();

        ASSERT_TRUE(converted_image.width() == 1920 && converted_image.height() == 1080);
    }

    // evaluate another image
    {
        cvpg::image_rgb_8bit image(1024, 768);

        auto promise_evaluate = std::make_shared<std::promise<cvpg::image_gray_8bit> >();
        auto future_evaluate = promise_evaluate->get_future();

        image_processor.evaluate(
            compile_id,
            std::move(image),
            [promise_evaluate](cvpg::imageproc::scripting::item item)
            {
                ASSERT_TRUE(item.type() == cvpg::imageproc::scripting::item::types::grayscale_8_bit_image);

                auto image = std::any_cast<cvpg::image_gray_8bit>(item.value());

                promise_evaluate->set_value(std::move(image));
            }
        );

        auto status = future_evaluate.wait_for(std::chrono::seconds(3));

        ASSERT_TRUE(status != std::future_status::deferred && status != std::future_status::timeout);

        auto converted_image = future_evaluate.get();

        ASSERT_TRUE(converted_image.width() == 1024 && converted_image.height() == 768);
    }
}
