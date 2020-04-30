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

TEST(test_scripting_algorithm_convert_to_gray, compile_valid_parameters)
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

    // good case
    {
        auto promise_compile = std::make_shared<std::promise<std::size_t> >();
        auto future_compile = promise_compile->get_future();

        image_processor.compile(
            R"(
                var input_rgb = input("rgb", 8)
                var input_gray = convert_to_gray(input_rgb, "use_red")
            )",
            [promise_compile](std::size_t compile_id)
            {
                promise_compile->set_value(compile_id);
            },
            [promise_compile](std::size_t compile_id, std::string error)
            {
                ASSERT_TRUE(!error.empty());
                ASSERT_TRUE(false);
            }
        );

        auto status = future_compile.wait_for(std::chrono::seconds(3));

        ASSERT_TRUE(status != std::future_status::deferred && status != std::future_status::timeout);
    }
}

TEST(test_scripting_algorithm_convert_to_gray, compile_invalid_parameters)
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

    // case: invalid convertion mode 'i dont know'
    {
        auto promise_compile = std::make_shared<std::promise<std::size_t> >();
        auto future_compile = promise_compile->get_future();

        image_processor.compile(
            R"(
                var input_rgb = input("rgb", 8)
                var input_gray = convert_to_gray(input_rgb, "i dont know")
            )",
            [promise_compile](std::size_t compile_id)
            {
                ASSERT_TRUE(false);
            },
            [promise_compile](std::size_t compile_id, std::string error)
            {
                ASSERT_TRUE(!error.empty());

                promise_compile->set_value(compile_id);
            }
        );

        auto status = future_compile.wait_for(std::chrono::seconds(3));

        ASSERT_TRUE(status != std::future_status::deferred && status != std::future_status::timeout);
    }

    // case: try to convert an already converted image
    {
        auto promise_compile = std::make_shared<std::promise<std::size_t> >();
        auto future_compile = promise_compile->get_future();

        image_processor.compile(
            R"(
                var input_rgb = input("rgb", 8)
                var input_gray = convert_to_gray(input_rgb, "use_red")
                var input_gray2 = convert_to_gray(input_gray, "use_red")
            )",
            [promise_compile](std::size_t compile_id)
            {
                ASSERT_TRUE(false);
            },
            [promise_compile](std::size_t compile_id, std::string error)
            {
                ASSERT_TRUE(!error.empty());

                promise_compile->set_value(compile_id);
            }
        );

        auto status = future_compile.wait_for(std::chrono::seconds(3));

        ASSERT_TRUE(status != std::future_status::deferred && status != std::future_status::timeout);
    }
}
