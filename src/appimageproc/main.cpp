#include <sys/ioctl.h>

#include <any>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <exception>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/asynchronous/servant_proxy.hpp>
#include <boost/asynchronous/trackable_servant.hpp>
#include <boost/asynchronous/diagnostics/formatter.hpp>
#include <boost/asynchronous/queue/lockfree_queue.hpp>
#include <boost/asynchronous/scheduler/multiqueue_threadpool_scheduler.hpp>
#include <boost/asynchronous/scheduler_shared_proxy.hpp>
#include <boost/asynchronous/scheduler/single_thread_scheduler.hpp>
#include <boost/asynchronous/scheduler/threadpool_scheduler.hpp>

#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/scripting/algorithm_set.hpp>
#include <libcvpg/imageproc/scripting/image_processor.hpp>
#include <libcvpg/imageproc/scripting/algorithms/base.hpp>
#include <libcvpg/imageproc/scripting/diagnostics/markdown_formatter.hpp>
#include <libcvpg/imageproc/scripting/diagnostics/typedefs.hpp>

#ifdef USE_TENSORFLOW_CC
#include <libcvpg/imageproc/algorithms/tfpredict.hpp>
#endif

int main(int argc, char * argv[])
{
    namespace po = boost::program_options;

    // general options
    std::string input_filename;
    std::string output_filename;
    std::string diagnostics_filename;
    std::uint32_t timeout = 10;
    bool quiet = false;

    // image processing options
    std::string script_filename;

#ifdef USE_TENSORFLOW_CC
    // TensorFlow inferencing options
    std::string tensorflow_model_path;
    std::string tensorflow_model_input;
    std::string tensorflow_model_outputs;
    std::string tensorflow_extract_outputs;
    std::string tensorflow_label_file;
#endif

    // performance options
    std::uint32_t xcutoff = 512;
    std::uint32_t ycutoff = 512;
    std::uint32_t threads = 0;

    // miscellaneous options
    std::size_t iterations = 1;

    // determine width of console
    struct winsize window;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);

    po::options_description general_options("general options", window.ws_col, window.ws_col / 2);
    general_options.add_options()
        ("help,h", "show this help text")
        ("input,i", po::value<std::string>(&input_filename), "filename of input image (8 bit grayscale or RGB PNG image)")
        ("output,o", po::value<std::string>(&output_filename)->default_value("output.png"), "filename of output image (8 bit grayscale or RGB PNG image)")
        ("diagnostics", po::value<std::string>(&diagnostics_filename), "filename where programm diagnostics (in 'Markdown' format) will be generated")
        ("timeout", po::value<std::uint32_t>(&timeout)->default_value(10), "timeout in seconds the processing will be aborted")
        ("quiet", "suppress all normal (non-error) outputs at console")
        ;

    po::options_description image_processing_options("image processing options", window.ws_col, window.ws_col / 2);
    image_processing_options.add_options()
        ("algorithms", "list of all available algorithms and their arguments")
        ("script,s", po::value<std::string>(&script_filename), "filename of image processing script")
        ;

#ifdef USE_TENSORFLOW_CC
    po::options_description tf_inferencing_options("TensorFlow inferencing options (for 'tfpredict')", window.ws_col, window.ws_col / 2);
    tf_inferencing_options.add_options()
        ("tfmodel", po::value<std::string>(&tensorflow_model_path), "path to TensorFlow model used at 'tfpredict' algorithm")
        ("tfinput", po::value<std::string>(&tensorflow_model_input), "name of input layer")
        ("tfoutputs", po::value<std::string>(&tensorflow_model_outputs), "comma separated list of output layers")
        ("tfextract", po::value<std::string>(&tensorflow_extract_outputs)->default_value("*"), "comma separated list of output descriptions to extract or '*' to extract all")
        ("tflabels", po::value<std::string>(&tensorflow_label_file), "file containing labels of detection classes")
#endif
        ;

    po::options_description performance_options("performance options", window.ws_col, window.ws_col / 2);
    performance_options.add_options()
        ("xcutoff", po::value<std::uint32_t>(&xcutoff)->default_value(512), "horizontal cutoff")
        ("ycutoff", po::value<std::uint32_t>(&ycutoff)->default_value(512), "vertical cutoff")
        ("threads", po::value<std::uint32_t>(&threads)->default_value(0), "amount of threads at threadpool (0 = all available)")
        ;

    po::options_description misc_options("miscellaneous options", window.ws_col, window.ws_col / 2);
    misc_options.add_options()
        ("iterations", po::value<std::size_t>(&iterations)->default_value(1), "amount of times the image filtering will performed successively")
        ;

    po::options_description cmdline_options("usage: imageproc [options]", window.ws_col, window.ws_col / 2);
    cmdline_options.add(general_options)
                   .add(image_processing_options)
#ifdef USE_TENSORFLOW_CC
                   .add(tf_inferencing_options)
#endif
                   .add(performance_options)
                   .add(misc_options);

    po::variables_map variables;
    po::store(po::parse_command_line(argc, argv, cmdline_options), variables);
    po::notify(variables);

    if (variables.count("help"))
    {
        std::cout << cmdline_options;
        return 1;
    }

    if (variables.count("algorithms"))
    {
        std::cout << "available algorithms" << std::endl
                  << "====================" << std::endl << std::endl;

        // create a map of algorithms by category
        std::map<std::string, std::vector<std::shared_ptr<cvpg::imageproc::scripting::algorithms::base> > > algorithms;

        for (auto const algorithm : cvpg::imageproc::scripting::algorithm_set().all())
        {
            algorithms[algorithm->category()].push_back(algorithm);
        }

        // print categories and their algorithms
        for (auto const & a : algorithms)
        {
            auto category = a.first;
            auto algs = a.second;

            std::cout << category << ":" << std::endl << std::endl;

            for (auto const & s : algs)
            {
                std::cout << "- " << s->name() << "(";

                if (!s->parameters().empty())
                {
                    decltype(s->parameters()) const & params = s->parameters();

                    std::stringstream ss;
                    ss << *(params.begin());

                    for (auto it = ++(params.begin()); it != params.end(); ++it)
                    {
                        ss << ", " << *it;
                    }

                    std::cout << ss.str();
                }

                std::cout << ")" << std::endl;
            }

            std::cout << std::endl;
        }

        std::cout << "example script" << std::endl
                  << "==============" << std::endl << std::endl;

        std::cout << "var input_rgb = input(\"rgb\", 8)" << std::endl
                  << "var input_gray = convert_to_gray(input_rgb, \"use_red\")" << std::endl;

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

    if (variables.count("diagnostics"))
    {
        diagnostics_filename = variables["diagnostics"].as<std::string>();
    }

    quiet = variables.count("quiet");

    if (variables.count("script"))
    {
        script_filename = variables["script"].as<std::string>();
    }
    else
    {
        std::cerr << "No script filename set." << std::endl;
        return 1;
    }

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

    if (variables.count("iterations"))
    {
        iterations = variables["iterations"].as<std::size_t>();
    }

    iterations = std::max<std::size_t>(1, iterations);

#ifdef USE_TENSORFLOW_CC
    if (variables.count("tfmodel"))
    {
        if (!variables.count("tfinput"))
        {
            std::cerr << "TensorFlow model set but no input layer set." << std::endl;
            return 1;
        }

        if (!variables.count("tfoutputs"))
        {
            std::cerr << "TensorFlow model set but no output layers set." << std::endl;
            return 1;
        }
    }

    // create a single-threaded world, where the TensorFlow processor will live inside
    auto tf_scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                            boost::asynchronous::single_thread_scheduler<
                                boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> > >(std::string("tfpredict_processor"));

    std::shared_ptr<cvpg::imageproc::algorithms::tfpredict_processor_proxy> tfpredict_processor;

    if (variables.count("tfmodel"))
    {
        tfpredict_processor = std::make_shared<cvpg::imageproc::algorithms::tfpredict_processor_proxy>(tf_scheduler);

        auto promise_tfmodel_load = std::make_shared<std::promise<bool> >();
        auto future_tfmodel_load = promise_tfmodel_load->get_future();

        tfpredict_processor->load_model(
            tensorflow_model_path,
            tensorflow_model_input,
            tensorflow_model_outputs,
            tensorflow_extract_outputs,
            [promise_tfmodel_load](bool status)
            {
                promise_tfmodel_load->set_value(status);
            }
        );

        // wait until TensorFlow model is loaded
        {
            auto status = future_tfmodel_load.wait_for(std::chrono::seconds(10)); // TODO make a parameter for timeout

            if (status == std::future_status::deferred)
            {
                std::cerr << "TensorFlow C++ model loading ended in deferred state. Abort" << std::endl;
                return 1;
            }
            else if (status == std::future_status::timeout)
            {
                std::cerr << "TensorFlow C++ model loading timed out. Abort" << std::endl;
                return 1;
            }

            auto tfmodel_load_result = future_tfmodel_load.get();

            if (!tfmodel_load_result)
            {
                std::cerr << "Error while loading TensorFlow model from directory '" << tensorflow_model_path << "'." << std::endl;
                return 1;
            }

            if (!quiet)
            {
                std::cout << "Using TensorFlow model from directory '" << tensorflow_model_path << "' for 'tfpredict' algorithm" << std::endl;
            }
        }

        // load label files
        std::unordered_map<std::size_t, std::string> labels;

        if (variables.count("tflabels"))
        {
            // read label file
            std::ifstream file(tensorflow_label_file);
            std::string labels_pbtxt { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
            file.close();

            if (!file.good())
            {
                std::cerr << "Error while loading label file '" << tensorflow_label_file << "'." << std::endl;
                return 1;
            }

            if (!quiet)
            {
                std::cout << "Loaded label file '" << tensorflow_label_file << "'" << std::endl;
            }

            // extract IDs and names from label string
            // TODO This is quite a hack for the moment! Replace this with a more robust implementation!
            try
            {
                std::vector<std::string> lines;
                boost::split(lines, labels_pbtxt, [](char c){ return c == '\n'; });

                for (std::size_t i = 0; i < (lines.size() - 5); i += 5)
                {
                    // extract ID
                    std::size_t pos = lines[i + 2].find_first_of(":");
                    std::size_t id = std::stoi(lines[i + 2].substr(pos + 2));

                    // extract name
                    pos = lines[i + 3].find_first_of(":");
                    std::string name = lines[i + 3].substr(pos + 2);
                    boost::algorithm::replace_first(name, "\"", "");
                    boost::algorithm::replace_last(name, "\"", "");

                    labels[id] = name;
                }

                if (!quiet)
                {
                    std::cout << "Extracted " << labels.size() << " classes from label file" << std::endl;
                }
            }
            catch (std::exception const & e)
            {
                std::cerr << "Error while parsing label file '" << tensorflow_label_file << "'. Error: " << e.what() << std::endl;
                return 1;
            }
            catch (...)
            {
                std::cerr << "Unknown error while parsing label file '" << tensorflow_label_file << "'." << std::endl;
                return 1;
            }

            if (tfpredict_processor)
            {
                tfpredict_processor->set_labels(std::move(labels));
            }
        }
        else
        {
            if (!quiet)
            {
                std::cout << "No label file set. Ignore classes in case of using 'tfpredict' algorithm" << std::endl;
            }
        }
    }
#endif

    // create a threadpool
    auto pool = boost::asynchronous::make_shared_scheduler_proxy<
                    boost::asynchronous::multiqueue_threadpool_scheduler<
                        boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> > >(threads, std::string("threadpool"));

    // a single-threaded world, where the image processor will live
    auto scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                         boost::asynchronous::single_thread_scheduler<
                             boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> > >(std::string("image_processor"));

    // formatter to produce diagnostics in markdown format
    auto formatter_scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                                   boost::asynchronous::single_thread_scheduler<
                                       boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> > >(std::string("formatter"));

    using formatter_type = cvpg::imageproc::scripting::diagnostics::markdown_formatter<>;

    boost::asynchronous::formatter_proxy<formatter_type> formatter(formatter_scheduler,
                                                                   pool,
                                                                   boost::asynchronous::make_scheduler_interfaces(scheduler, pool, formatter_scheduler));

    if (!quiet)
    {
        std::cout << "Using threadpool with " << threads << " worker threads" << std::endl;
    }

    // read script file
    std::ifstream file(script_filename);
    std::string script { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
    file.close();

    if (!file.good())
    {
        std::cerr << "Error while loading script '" << script_filename << "'." << std::endl;
        return 1;
    }

    if (!quiet)
    {
        std::cout << "Loaded script '" << script_filename << "'" << std::endl;
    }

    // create the image processor
    cvpg::imageproc::scripting::image_processor_proxy processor(scheduler, pool);

    boost::asynchronous::formatter_proxy<formatter_type> diagnostics_formatter(formatter_scheduler,
                                                                               pool,
                                                                               boost::asynchronous::make_scheduler_interfaces(scheduler, pool, formatter_scheduler));

#ifdef USE_TENSORFLOW_CC
    if (tfpredict_processor)
    {
        processor.add_param("tfmodel_processor", tfpredict_processor);
    }
#endif

    // set cutoff parameters
    processor.add_param("cutoff_x", xcutoff);
    processor.add_param("cutoff_y", ycutoff);

    // compiling script
    struct compile_result
    {
        std::size_t id = 0;
        std::string error;
    };

    auto promise_compile = std::make_shared<std::promise<compile_result> >();
    auto future_compile = promise_compile->get_future();

    processor.compile(script,
                      [promise_compile](std::size_t compile_id)
                      {
                          promise_compile->set_value({ compile_id });
                      },
                      [promise_compile](std::size_t compile_id, std::string error)
                      {
                          promise_compile->set_value({ compile_id, std::move(error) });
                      });

    auto status = future_compile.wait_for(std::chrono::seconds(3));

    if (status == std::future_status::deferred)
    {
        std::cerr << "Script compiling ended in deferred state. Abort" << std::endl;
        return 1;
    }
    else if (status == std::future_status::timeout)
    {
        std::cerr << "Script compiling timed out. Abort" << std::endl;
        return 1;
    }

    auto compile_result = future_compile.get();

    // check if script compiled successfully
    if (!compile_result.error.empty())
    {
        std::cerr << "Script compiled with error '" << compile_result.error << "'. Abort" << std::endl;
        return 1;
    }

    if (!quiet)
    {
        std::cout << "Script compiled" << std::endl;
    }

    // read input image
    std::uint8_t channels = 0;
    std::any png;

    try
    {
        auto [ c, p ] = cvpg::read_png(input_filename);

        channels = c;
        png = std::move(p);
    }
    catch (std::exception const & e)
    {
        std::cerr << "Error while reading PNG image. Error: '" << e.what() << "'" << std::endl;
        return 1;
    }

    // evaluate image (with multiple iterations)
    std::vector<std::chrono::milliseconds> durations;
    durations.reserve(iterations);

    cvpg::imageproc::scripting::item result;

    for (std::size_t i = 0; i < iterations; ++i)
    {
        // create a shared promise used by the callback inside the image processor to finish the processing
        auto promise_evaluate = std::make_shared<std::promise<cvpg::imageproc::scripting::item> >();
        auto future_evaluate = promise_evaluate->get_future();

        // start measure time
        auto start = std::chrono::steady_clock::now();

        try
        {
            if (channels == 1)
            {
                auto image = std::any_cast<cvpg::image_gray_8bit>(png);

                if (!quiet && i == 0)
                {
                    std::cout << "Loaded grayscale image with " << image.width() << "x" << image.height() << " pixels" << std::endl;
                }

                processor.evaluate(compile_result.id,
                                std::move(image),
                                [promise_evaluate](cvpg::imageproc::scripting::item result)
                                {
                                    promise_evaluate->set_value(std::move(result));
                                });
            }
            else if (channels == 3)
            {
                auto image = std::any_cast<cvpg::image_rgb_8bit>(png);

                if (!quiet && i == 0)
                {
                    std::cout << "Loaded RGB image with " << image.width() << "x" << image.height() << " pixels" << std::endl;
                }

                processor.evaluate(compile_result.id,
                                std::move(image),
                                [promise_evaluate](cvpg::imageproc::scripting::item result)
                                {
                                    promise_evaluate->set_value(std::move(result));
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
            std::cerr << "Error while processing PNG image. Error: '" << e.what() << "'" << std::endl;
            return 1;
        }

        status = future_evaluate.wait_for(std::chrono::seconds(timeout));

        // stop time measurement
        auto stop = std::chrono::steady_clock::now();

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

        durations.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(stop - start));

        if (i == (iterations - 1))
        {
            result = future_evaluate.get();
        }
    }

    if (!quiet)
    {
        auto sum = std::accumulate(durations.begin(),
                                   durations.end(),
                                   static_cast<std::size_t>(0),
                                   [](std::size_t sum, auto duration)
                                   {
                                       return sum + duration.count();
                                   });

        std::cout << "Image processing done in " << (sum / static_cast<double>(iterations)) << " ms" << std::endl;
    }

    if (result.type() == cvpg::imageproc::scripting::item::types::grayscale_8_bit_image)
    {
        cvpg::write_png(std::any_cast<cvpg::image_gray_8bit>(result.value()), output_filename);
    }
    else if (result.type() == cvpg::imageproc::scripting::item::types::rgb_8_bit_image)
    {
        cvpg::write_png(std::any_cast<cvpg::image_rgb_8bit>(result.value()), output_filename);
    }
    else
    {
        std::cerr << "Cannot write unsupported image type '" << result.type() << "' to file '" << output_filename << "'." << std::endl;
        return 1;
    }

    if (!quiet)
    {
        std::cout << "Saved result to file '" << output_filename << "'" << std::endl;
    }

    std::stringstream diagnostics_stream;

    if (!diagnostics_filename.empty())
    {
        std::ofstream out(diagnostics_filename);
        out << diagnostics_formatter.format().get();
        out.close();

        if (!quiet)
        {
            std::cerr << "Diagnostics saved at '" << diagnostics_filename << "'" << std::endl;
        }
    }

    return 0;
}
