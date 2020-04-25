#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <thread>

#include <boost/asynchronous/queue/lockfree_queue.hpp>
#include <boost/asynchronous/scheduler_shared_proxy.hpp>
#include <boost/asynchronous/scheduler/multiqueue_threadpool_scheduler.hpp>
#include <boost/asynchronous/scheduler/single_thread_scheduler.hpp>

#include <libcvpg/imageproc/scripting/image_processor.hpp>
#include <libcvpg/imageproc/scripting/diagnostics/typedefs.hpp>

TEST(test_scripting_algorithm_sobel, compile_invalid_parameters)
{
    // create a thread pool for a single thread
    auto pool = boost::asynchronous::make_shared_scheduler_proxy<
                    boost::asynchronous::multiqueue_threadpool_scheduler<
                        boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> > >(1, std::string("threadpool"));

    // create image processor
    auto scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                        boost::asynchronous::single_thread_scheduler<
                            boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> > >(std::string("image_processor"));

    cvpg::imageproc::scripting::image_processor_proxy image_processor(scheduler, pool);

    // case: invalid filter width (!= 3)
    {
        auto promise_compile = std::make_shared<std::promise<std::size_t> >();
        auto future_compile = promise_compile->get_future();

        image_processor.compile(
            R"(
                var input_rgb = input("rgb", 8)
                var edges = sobel(input_rgb, 4, "hor", "ignore")
            )",
            [promise_compile](bool successful, std::size_t compile_id)
            {
                ASSERT_TRUE(!successful);

                promise_compile->set_value(compile_id);
            }
        );

        auto status = future_compile.wait_for(std::chrono::seconds(3));

        ASSERT_TRUE(status != std::future_status::deferred && status != std::future_status::timeout);
    }

    // case: invalid operation mode
    {
        auto promise_compile = std::make_shared<std::promise<std::size_t> >();
        auto future_compile = promise_compile->get_future();

        image_processor.compile(
            R"(
                var input_rgb = input("rgb", 8)
                var edges = sobel(input_rgb, 3, "horizontal", "ignore")
            )",
            [promise_compile](bool successful, std::size_t compile_id)
            {
                ASSERT_TRUE(!successful);

                promise_compile->set_value(compile_id);
            }
        );

        auto status = future_compile.wait_for(std::chrono::seconds(3));

        ASSERT_TRUE(status != std::future_status::deferred && status != std::future_status::timeout);
    }

    // case: invalid border mode
    {
        auto promise_compile = std::make_shared<std::promise<std::size_t> >();
        auto future_compile = promise_compile->get_future();

        image_processor.compile(
            R"(
                var input_rgb = input("rgb", 8)
                var edges = sobel(input_rgb, 3, "hor", "do some magic")
            )",
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
