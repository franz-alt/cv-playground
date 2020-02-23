#include <sys/ioctl.h>

#include <any>
#include <cstdint>
#include <exception>
#include <functional>
#include <future>
#include <iostream>
#include <string>
#include <thread>

#include <boost/program_options.hpp>

#include <boost/asynchronous/servant_proxy.hpp>
#include <boost/asynchronous/trackable_servant.hpp>
#include <boost/asynchronous/queue/lockfree_queue.hpp>
#include <boost/asynchronous/scheduler/multiqueue_threadpool_scheduler.hpp>
#include <boost/asynchronous/scheduler_shared_proxy.hpp>
#include <boost/asynchronous/scheduler/single_thread_scheduler.hpp>
#include <boost/asynchronous/scheduler/threadpool_scheduler.hpp>

#include <libcvpg/core/image.hpp>

class imageprocessor : public boost::asynchronous::trackable_servant<>
{
public:
    imageprocessor(boost::asynchronous::any_weak_scheduler<> scheduler, boost::asynchronous::any_shared_scheduler_proxy<> pool)
        : boost::asynchronous::trackable_servant<>(scheduler, pool)
    {}

    void process(cvpg::image_gray_8bit image, std::function<void(std::any)> callback)
    {
        // TODO implement me
    }

    void process(cvpg::image_rgb_8bit image, std::function<void(std::any)> callback)
    {
        // TODO implement me
    }
};

class imageprocessor_proxy : public boost::asynchronous::servant_proxy<imageprocessor_proxy, imageprocessor>
{
public:
    template<typename... Args>
    imageprocessor_proxy(Args... args)
        : boost::asynchronous::servant_proxy<imageprocessor_proxy, imageprocessor>(args...)
    {}

    BOOST_ASYNC_POST_MEMBER(process, 1)
};

int main(int argc, char * argv[])
{
    namespace po = boost::program_options;

    // general options
    std::string input_filename;
    std::string output_filename;
    std::uint32_t timeout = 10;
    bool quiet = false;

    // image processing options
    std::string filter_expression;

    // performance options
    std::uint32_t xcutoff = 512;
    std::uint32_t ycutoff = 512;
    std::uint32_t threads = 0;

    // determine width of console
    struct winsize window;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);

    po::options_description general_options("general options", window.ws_col, window.ws_col / 2);
    general_options.add_options()
        ("help,h", "show this help text")
        ("input,i", po::value<std::string>(&input_filename), "filename of input image (8 bit grayscale or RGB PNG image)")
        ("output,o", po::value<std::string>(&output_filename)->default_value("output.png"), "filename of output image (8 bit grayscale or RGB PNG image)")
        ("timeout", po::value<std::uint32_t>(&timeout)->default_value(10), "timeout in seconds the processing will be aborted")
        ("quiet", "suppress all normal (non-error) outputs at console")
        ;

    po::options_description image_processing_options("image processing options", window.ws_col, window.ws_col / 2);
    image_processing_options.add_options()
        ("filters", "list of all available filters and their arguments")
        ("expression", po::value<std::string>(&filter_expression), "filter expression in form '<name>(<argument1>, <argument2>, ...)'")
        ;

    po::options_description performance_options("performance options", window.ws_col, window.ws_col / 2);
    performance_options.add_options()
        ("xcutoff", po::value<std::uint32_t>(&xcutoff)->default_value(512), "horizontal cutoff")
        ("ycutoff", po::value<std::uint32_t>(&ycutoff)->default_value(512), "vertical cutoff")
        ("threads", po::value<std::uint32_t>(&threads)->default_value(0), "amount of threads at threadpool (0 = all available)")
        ;

    po::options_description cmdline_options("usage: imageproc [options]", window.ws_col, window.ws_col / 2);
    cmdline_options.add(general_options)
                   .add(image_processing_options)
                   .add(performance_options);

    po::variables_map variables;
    po::store(po::parse_command_line(argc, argv, cmdline_options), variables);
    po::notify(variables);

    if (variables.count("help"))
    {
        std::cout << cmdline_options;
        return 1;
    }

    if (variables.count("input"))
    {
        input_filename = variables["input"].as<std::string>();
    }
    else
    {
        std::cerr << "No input image set." << std::endl;
        return 1;
    }

    output_filename = variables["output"].as<std::string>();
    quiet = variables.count("quiet");

    if (variables.count("threads"))
    {
        threads = variables["threads"].as<std::uint32_t>();

        if (threads == 0)
        {
            threads = std::thread::hardware_concurrency();
        }
    }
    else
    {
        threads = std::thread::hardware_concurrency();
    }

    // a single-threaded world, where the imageprocessor will live
    auto scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                         boost::asynchronous::single_thread_scheduler<
                             boost::asynchronous::lockfree_queue<> > >(std::string("imageprocessor"));

    // create a threadpool
    auto pool = boost::asynchronous::make_shared_scheduler_proxy<
                    boost::asynchronous::multiqueue_threadpool_scheduler<
                        boost::asynchronous::lockfree_queue<> > >(threads, std::string("threadpool"));

    if (!quiet)
    {
        std::cout << "Using threadpool with " << threads << " worker threads" << std::endl;
    }

    // create the imageprocessor
    imageprocessor_proxy processor(scheduler, pool);

    // create a shared promise used by the callback inside the imageprocessor to finish the processing
    auto promise = std::make_shared<std::promise<void> >();
    auto future = promise->get_future();

    try
    {
        auto [ channels, png ] = cvpg::read_png(input_filename);

        if (channels == 1)
        {
            auto image = std::any_cast<cvpg::image_gray_8bit>(png);

            if (!quiet)
            {
                std::cout << "Loaded grayscale image with " << image.width() << "x" << image.height() << " pixels" << std::endl;
            }

            processor.process(std::move(image),
                              [promise](std::any result)
                              {
                                  promise->set_value();
                              });
        }
        else if (channels == 3)
        {
            auto image = std::any_cast<cvpg::image_rgb_8bit>(png);

            if (!quiet)
            {
                std::cout << "Loaded RGB image with " << image.width() << "x" << image.height() << " pixels" << std::endl;
            }

            processor.process(std::move(image),
                              [promise](std::any result)
                              {
                                  promise->set_value();
                              });
        }
        else
        {
            std::cerr << "Only grayscale or RGB images are supported. Abort" << std::endl;
            return 1;
        }
    }
    catch (std::exception const & e)
    {
        std::cerr << "Error while reading PNG image. Error: '" << e.what() << "'" << std::endl;
        return 1;
    }

    auto status = future.wait_for(std::chrono::seconds(timeout));

    if (status == std::future_status::deferred)
    {
        std::cerr << "Image processing ended in deferred state. Abort" << std::endl;
        return 1;
    }
    else if (status == std::future_status::timeout)
    {
        std::cerr << "Image processing timed out. Abort" << std::endl;
        return 1;
    }

    if (!quiet)
    {
        std::cout << "Image processing done" << std::endl;
    }

    return 0;
}
