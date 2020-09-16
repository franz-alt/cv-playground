#include <sys/ioctl.h>

#include <any>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <boost/program_options.hpp>

#include <boost/asynchronous/scheduler_shared_proxy.hpp>
#include <boost/asynchronous/diagnostics/formatter.hpp>
#include <boost/asynchronous/scheduler/multiqueue_threadpool_scheduler.hpp>

#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/scripting/image_processor.hpp>
#include <libcvpg/imageproc/scripting/diagnostics/markdown_formatter.hpp>
#include <libcvpg/imageproc/scripting/diagnostics/typedefs.hpp>
#include <libcvpg/videoproc/pipelines/file_to_file.hpp>

#include "progress_monitor.hpp"

extern "C" {
#include <libavutil/avutil.h>
}

static void avlog_cb(void *, int level, const char * szFmt, va_list varg)
{
    // TOOD collect data and present it (somehow) to the user
}

int main(int argc, char * argv[])
{
    namespace po = boost::program_options;

    // general options
    std::string input_filename;
    std::string output_filename;
    std::string diagnostics_filename;
    std::uint32_t timeout = 60;
    bool quiet = false;

    // video processing options
    std::string frame_script_filename;
    std::string interframe_script_filename;
    std::size_t buffered_input_frames = 20;
    std::size_t buffered_processing_frames = 50;
    std::size_t buffered_output_frames = 20;

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
        ("input,i", po::value<std::string>(&input_filename), "filename of input video (MP4 format supported only)")
        ("output,o", po::value<std::string>(&output_filename)->default_value("output.mp4"), "filename of output video")
        ("diagnostics", po::value<std::string>(&diagnostics_filename), "filename where programm diagnostics (in 'Markdown' format) will be generated")
        ("timeout", po::value<std::uint32_t>(&timeout)->default_value(10), "timeout in seconds the processing will be aborted")
        ("quiet", "suppress all normal (non-error) outputs at console")
        ;

    po::options_description video_processing_options("video processing options", window.ws_col, window.ws_col / 2);
    video_processing_options.add_options()
        ("frame-script", po::value<std::string>(&frame_script_filename), "name of script file that should be processed for each frame")
        ("interframe-script", po::value<std::string>(&interframe_script_filename), "name of script file that should be processed for each pair of input images")
        ("input-buffer", po::value<std::size_t>(&buffered_input_frames)->default_value(50), "amount of buffered frames when reading video frames")
        ("packet-buffer", po::value<std::size_t>(&buffered_processing_frames)->default_value(50), "amount of buffered frames at each processing stage (minimum size = 3)")
        ("output-buffer", po::value<std::size_t>(&buffered_output_frames)->default_value(50), "amount of buffered frames when writing video frames")
        ;

    po::options_description performance_options("performance options", window.ws_col, window.ws_col / 2);
    performance_options.add_options()
        ("xcutoff", po::value<std::uint32_t>(&xcutoff)->default_value(512), "horizontal cutoff")
        ("ycutoff", po::value<std::uint32_t>(&ycutoff)->default_value(512), "vertical cutoff")
        ("threads", po::value<std::uint32_t>(&threads)->default_value(0), "amount of threads at threadpool (0 = all available)")
        ;

    po::options_description cmdline_options("usage: videoproc [options]", window.ws_col, window.ws_col / 2);
    cmdline_options.add(general_options)
                   .add(video_processing_options)
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
        std::cerr << "No input video set." << std::endl;
        return 1;
    }

    output_filename = variables["output"].as<std::string>();

    if (variables.count("diagnostics"))
    {
        diagnostics_filename = variables["diagnostics"].as<std::string>();
    }

    quiet = variables.count("quiet");

    if (variables.count("frame-script"))
    {
        frame_script_filename = variables["frame-script"].as<std::string>();
    }
    else
    {
        std::cerr << "No frame script filename set." << std::endl;
        return 1;
    }

    if (variables.count("interframe-script"))
    {
        interframe_script_filename = variables["interframe-script"].as<std::string>();
    }
    else
    {
        std::cerr << "No inter-frame script filename set." << std::endl;
        return 1;
    }

    if (buffered_processing_frames < 3)
    {
        std::cerr << "The internal packet buffer size must contain at least three entries." << std::endl;
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

    // create a thread pool
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

    // read frame script file
    std::ifstream frame_script_file(frame_script_filename);
    std::string frame_script { std::istreambuf_iterator<char>(frame_script_file), std::istreambuf_iterator<char>() };
    frame_script_file.close();

    if (!frame_script_file.good())
    {
        std::cerr << "Error while loading frame script '" << frame_script_filename << "'." << std::endl;
        return 1;
    }

    if (!quiet)
    {
        std::cout << "Loaded frame script '" << frame_script_filename << "'" << std::endl;
    }

    // read inter-frame script file
    std::ifstream interframe_script_file(interframe_script_filename);
    std::string interframe_script { std::istreambuf_iterator<char>(interframe_script_file), std::istreambuf_iterator<char>() };
    interframe_script_file.close();

    if (!interframe_script_file.good())
    {
        std::cerr << "Error while loading inter-frame script '" << interframe_script_filename << "'." << std::endl;
        return 1;
    }

    if (!quiet)
    {
        std::cout << "Loaded inter-frame script '" << interframe_script_filename << "'" << std::endl;
    }

    // use own logging callback
    av_log_set_callback(avlog_cb);

    // create the image processor
    auto image_processor_scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                                         boost::asynchronous::single_thread_scheduler<
                                             boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> > >(std::string("image_processor"));

    boost::asynchronous::formatter_proxy<formatter_type> diagnostics_formatter(formatter_scheduler,
                                                                               pool,
                                                                               boost::asynchronous::make_scheduler_interfaces(scheduler, pool, formatter_scheduler));

    cvpg::imageproc::scripting::image_processor_proxy image_processor(image_processor_scheduler, pool);

    // set cutoff parameters
    image_processor.add_param("cutoff_x", xcutoff);
    image_processor.add_param("cutoff_y", ycutoff);

    struct compile_result
    {
        std::size_t id = 0;
        std::string error;
    };

    // compiling frame script
    {
        auto promise_compile = std::make_shared<std::promise<compile_result> >();
        auto future_compile = promise_compile->get_future();

        image_processor.compile(
            frame_script,
            [promise_compile](std::size_t compile_id)
            {
                promise_compile->set_value({ compile_id });
            },
            [promise_compile](std::size_t compile_id, std::string error)
            {
                promise_compile->set_value({ compile_id, std::move(error) });
            }
        );

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
            std::cerr << "Frame script compiled with error '" << compile_result.error << "'. Abort" << std::endl;
            return 1;
        }

        if (!quiet)
        {
            std::cout << "Frame script compiled" << std::endl;
        }
    }

    // compiling inter-frame script
    {
        auto promise_compile = std::make_shared<std::promise<compile_result> >();
        auto future_compile = promise_compile->get_future();

        image_processor.compile(
            interframe_script,
            [promise_compile](std::size_t compile_id)
            {
                promise_compile->set_value({ compile_id });
            },
            [promise_compile](std::size_t compile_id, std::string error)
            {
                promise_compile->set_value({ compile_id, std::move(error) });
            }
        );

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
            std::cerr << "Inter-frame script compiled with error '" << compile_result.error << "'. Abort" << std::endl;
            return 1;
        }

        if (!quiet)
        {
            std::cout << "Inter-frame script compiled" << std::endl;
        }
    }

    // create a video file reader
    auto file_in_scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                                    boost::asynchronous::single_thread_scheduler<
                                        boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> > >();

    auto file_reader = std::make_shared<cvpg::videoproc::sources::image_rgb_8bit_file_proxy>(file_in_scheduler, buffered_input_frames);

    // create a frame and interframe processor
    auto processors_scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                                    boost::asynchronous::single_thread_scheduler<
                                        boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> > >(std::string("processors"));

    auto frame_processor = std::make_shared<cvpg::videoproc::processors::image_rgb_8bit_frame_proxy>(processors_scheduler, pool, buffered_processing_frames, image_processor);
    auto interframe_processor = std::make_shared<cvpg::videoproc::processors::image_rgb_8bit_interframe_proxy>(processors_scheduler, pool, buffered_processing_frames, image_processor);

    // create a video file producer
    auto file_out_scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                                    boost::asynchronous::single_thread_scheduler<
                                        boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> > >();

    auto file_producer = std::make_shared<cvpg::videoproc::sinks::image_rgb_8bit_file_proxy>(file_out_scheduler, buffered_output_frames);

    // create an progress monitor
    auto progress_monitor_scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                                          boost::asynchronous::single_thread_scheduler<
                                              boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> > >();

    auto progress_monitor = std::make_shared<progress_monitor_proxy>(progress_monitor_scheduler, !quiet);

    // create a pipeline for the stages
    auto pipeline_scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                                    boost::asynchronous::single_thread_scheduler<
                                        boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> > >();

    auto promise_pipeline = std::make_shared<std::promise<std::string> >();

    auto pipeline = std::make_shared<cvpg::videoproc::pipelines::image_rgb_8bit_file_to_file_proxy>(pipeline_scheduler, file_reader, frame_processor, interframe_processor, file_producer);

    pipeline->start(
        input_filename,
        output_filename,
        frame_script,
        interframe_script,
        [promise_pipeline]()
        {
            promise_pipeline->set_value("");
        },
        [progress_monitor](std::size_t context_id, std::int64_t frames)
        {
            progress_monitor->init(context_id, frames);
        },
        [promise_pipeline](std::size_t context_id, std::string error)
        {
            promise_pipeline->set_value(std::move(error));
        },
        [progress_monitor](std::size_t context_id, cvpg::videoproc::update_indicator update) mutable
        {
            progress_monitor->update(context_id, std::move(update));
        }
    );

    auto future_pipeline = promise_pipeline->get_future();

    auto status = future_pipeline.wait_for(std::chrono::seconds(timeout));

    bool finished_with_errors = false;

    if (status == std::future_status::deferred)
    {
        std::cerr << "Execution aborted in defered state" << std::endl;

        finished_with_errors = true;
    }
    else if (status == std::future_status::timeout)
    {
        std::cerr << "Execution timed out" << std::endl;

        finished_with_errors = true;
    }
    else
    {
        auto error = future_pipeline.get();

        if (error.empty() && !quiet)
        {
            std::cout << "Processing done" << std::endl;
        }
        else if (!error.empty())
        {
            std::cerr << "Processing failed. Error: '" << error << "'" << std::endl;
        }
    }

    std::stringstream diagnostics_stream;

    if (!diagnostics_filename.empty())
    {
        std::ofstream out(diagnostics_filename);
        out << diagnostics_formatter.format().get();
        out.close();

        if (!quiet)
        {
            std::cout << "Diagnostics saved at '" << diagnostics_filename << "'" << std::endl;
        }
    }

    return finished_with_errors ? 1 : 0;
}
