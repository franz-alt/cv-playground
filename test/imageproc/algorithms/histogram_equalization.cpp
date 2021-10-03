#include <gtest/gtest.h>

#include <chrono>
#include <exception>
#include <memory>
#include <thread>

#include <boost/asynchronous/servant_proxy.hpp>
#include <boost/asynchronous/trackable_servant.hpp>
#include <boost/asynchronous/queue/lockfree_queue.hpp>
#include <boost/asynchronous/scheduler_shared_proxy.hpp>
#include <boost/asynchronous/scheduler/multiqueue_threadpool_scheduler.hpp>
#include <boost/asynchronous/scheduler/single_thread_scheduler.hpp>

#include <libcvpg/core/histogram.hpp>
#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/algorithms/histogram_equalization.hpp>

struct test_servant : boost::asynchronous::trackable_servant<>
{
    test_servant(boost::asynchronous::any_weak_scheduler<> scheduler, boost::asynchronous::any_shared_scheduler_proxy<> pool)
        : boost::asynchronous::trackable_servant<>(scheduler, pool)
    {}

    std::future<cvpg::image_gray_8bit> histogram_equalization(cvpg::image_gray_8bit image)
    {
        auto promise_alg = std::make_shared<std::promise<cvpg::image_gray_8bit> >();
        auto future_alg = promise_alg->get_future();

        post_callback(
            [img = std::move(image)]()
            {
                return cvpg::imageproc::algorithms::histogram_equalization(std::move(img));
            },
            [promise_alg](auto cont_res)
            {
                try
                {
                    promise_alg->set_value(std::move(cont_res.get()));
                }
                catch (...)
                {
                    promise_alg->set_exception(std::current_exception());
                }
            }
        );

        return future_alg;
    }
};

struct test_servant_proxy : public boost::asynchronous::servant_proxy<test_servant_proxy, test_servant>
{
   template<typename... Args>
   test_servant_proxy(Args... args)
       : boost::asynchronous::servant_proxy<test_servant_proxy, test_servant>(args...)
   {}

   BOOST_ASYNC_FUTURE_MEMBER(histogram_equalization)
};

TEST(test_algorithms, histogram_equalization)
{
    // create a thread pool for a single thread
    auto pool = boost::asynchronous::make_shared_scheduler_proxy<
                    boost::asynchronous::multiqueue_threadpool_scheduler<
                        boost::asynchronous::lockfree_queue<> > >(1);

    // create a scheduler
    auto scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                        boost::asynchronous::single_thread_scheduler<
                            boost::asynchronous::lockfree_queue<> > >();

    // create test image
    cvpg::image_gray_8bit image(8, 8);

    auto * raw = image.data(0).get();

    // fill with medium gray value of 128 ...
    memset(raw, 128, image.width() * image.height());

    // ... and set some selected values
    // (taken from Wikipedia 'Histogram equalization' at https://en.wikipedia.org/wiki/Histogram_equalization)
    raw[0] = 52;
    raw[1] = 55;
    raw[2] = 55;
    raw[3] = 55;
    raw[4] = 58;
    raw[5] = 58;
    raw[6] = 59;
    raw[7] = 59;
    raw[8] = 59;
    raw[9] = 60;

    test_servant_proxy tester(scheduler, pool);
    auto f = tester.histogram_equalization(std::move(image));

    try
    {
        auto status = f.wait_for(std::chrono::seconds(3));

        ASSERT_TRUE(status != std::future_status::deferred && status != std::future_status::timeout);

        auto image = f.get().get();
    
        auto * raw = image.data(0).get();

        // (taken from Wikipedia 'Histogram equalization' at https://en.wikipedia.org/wiki/Histogram_equalization)
        ASSERT_TRUE(raw[0] == 0);
        ASSERT_TRUE(raw[1] == 12);
        ASSERT_TRUE(raw[4] == 20);
        ASSERT_TRUE(raw[6] == 32);
    }
    catch(std::exception const & e)
    {
        std::cerr << e.what() << std::endl << std::flush;

        ASSERT_TRUE(false);
    }
    catch(...)
    {
        ASSERT_TRUE(false);
    }
}
