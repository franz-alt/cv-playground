#include <gtest/gtest.h>

#include <chrono>
#include <exception>
#include <memory>
#include <thread>
#include <vector>

#include <boost/asynchronous/servant_proxy.hpp>
#include <boost/asynchronous/trackable_servant.hpp>
#include <boost/asynchronous/queue/lockfree_queue.hpp>
#include <boost/asynchronous/scheduler_shared_proxy.hpp>
#include <boost/asynchronous/scheduler/multiqueue_threadpool_scheduler.hpp>
#include <boost/asynchronous/scheduler/single_thread_scheduler.hpp>

#include <libcvpg/core/histogram.hpp>
#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/algorithms/hog.hpp>

template<class image_type>
struct test_servant : boost::asynchronous::trackable_servant<>
{
    test_servant(boost::asynchronous::any_weak_scheduler<> scheduler, boost::asynchronous::any_shared_scheduler_proxy<> pool)
        : boost::asynchronous::trackable_servant<>(scheduler, pool)
    {}

    std::future<std::vector<cvpg::histogram<double> > > hog(image_type image)
    {
        auto promise_alg = std::make_shared<std::promise<std::vector<cvpg::histogram<double> > > >();
        auto future_alg = promise_alg->get_future();

        post_callback(
            [img = std::move(image)]() mutable
            {
                return cvpg::imageproc::algorithms::hog(std::move(img));
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

    std::future<cvpg::image_gray_8bit> hog_image(std::vector<cvpg::histogram<double> > histograms, std::size_t cells_per_row, std::size_t cell_dimension)
    {
        auto promise_alg = std::make_shared<std::promise<cvpg::image_gray_8bit> >();
        auto future_alg = promise_alg->get_future();

        post_callback(
            [histograms = std::move(histograms), cells_per_row, cell_dimension]() mutable
            {
                return cvpg::imageproc::algorithms::hog_image(std::move(histograms), cells_per_row, cell_dimension);
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

template<class image_type>
struct test_servant_proxy : public boost::asynchronous::servant_proxy<test_servant_proxy<image_type>, test_servant<image_type> >
{
   template<typename... Args>
   test_servant_proxy(Args... args)
       : boost::asynchronous::servant_proxy<test_servant_proxy<image_type>, test_servant<image_type> >(std::forward<Args>(args)...)
   {}

   BOOST_ASYNC_FUTURE_MEMBER(hog)
   BOOST_ASYNC_FUTURE_MEMBER(hog_image)
};

TEST(test_algorithms, hog_gray_8bit)
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
    cvpg::image_gray_8bit image(128, 128);

    test_servant_proxy<decltype(image)> tester(scheduler, pool);

    try
    {
        auto f = tester.hog(std::move(image));

        auto status = f.wait_for(std::chrono::seconds(3));

        ASSERT_TRUE(status != std::future_status::deferred && status != std::future_status::timeout);

        auto histograms = std::move(f.get().get());

        // test for the correct amount of histogram feature vectors
        ASSERT_EQ(histograms.size(), (image.width() / 8) * (image.height() / 8));
    }
    catch (std::exception const & e)
    {
        std::cerr << e.what() << std::endl << std::flush;

        ASSERT_TRUE(false);
    }
    catch(...)
    {
        ASSERT_TRUE(false);
    }
}
