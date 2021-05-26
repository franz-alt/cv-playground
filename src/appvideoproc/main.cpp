#include <sys/ioctl.h>

#include <any>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <string>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/asynchronous/scheduler_shared_proxy.hpp>
#include <boost/asynchronous/diagnostics/formatter.hpp>
#include <boost/asynchronous/scheduler/multiqueue_threadpool_scheduler.hpp>

#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/scripting/image_processor.hpp>
#include <libcvpg/imageproc/scripting/diagnostics/markdown_formatter.hpp>
#include <libcvpg/imageproc/scripting/diagnostics/typedefs.hpp>
#include <libcvpg/videoproc/any_stage.hpp>
#include <libcvpg/videoproc/pipelines/any_pipeline.hpp>
#include <libcvpg/videoproc/pipelines/file_to_file.hpp>
#include <libcvpg/videoproc/pipelines/rtsp_to_file.hpp>

#ifdef USE_TENSORFLOW_CC
#include <libcvpg/imageproc/algorithms/tfpredict.hpp>
#endif

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
    std::string input_uri;
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

#ifdef USE_TENSORFLOW_CC
    // TensorFlow inferencing options
    std::string tensorflow_model_path;
    std::string tensorflow_model_input;
    std::string tensorflow_model_outputs;
    std::string tensorflow_extract_outputs;
    std::string tensorflow_label_file;
    std::uint32_t tensorflow_threads = 0;
#endif

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
        ("input,i", po::value<std::string>(&input_uri), "filename of input video (MP4 format supported only) or URI of RTSP stream")
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
        ("processing-buffer", po::value<std::size_t>(&buffered_processing_frames)->default_value(50), "amount of buffered frames at each processing stage (minimum size is size of input buffer)")
        ("output-buffer", po::value<std::size_t>(&buffered_output_frames)->default_value(50), "amount of buffered frames when writing video frames")
        ;

#ifdef USE_TENSORFLOW_CC
    po::options_description tf_inferencing_options("TensorFlow inferencing options (for 'tfpredict')", window.ws_col, window.ws_col / 2);
    tf_inferencing_options.add_options()
        ("tfmodel", po::value<std::string>(&tensorflow_model_path), "path to TensorFlow model used at 'tfpredict' algorithm")
        ("tfinput", po::value<std::string>(&tensorflow_model_input), "name of input layer")
        ("tfoutputs", po::value<std::string>(&tensorflow_model_outputs), "comma separated list of output layers")
        ("tfextract", po::value<std::string>(&tensorflow_extract_outputs)->default_value("*"), "comma separated list of output descriptions to extract or '*' to extract all")
        ("tflabels", po::value<std::string>(&tensorflow_label_file), "file containing labels of detection classes")
        ("tfthreads", po::value<std::uint32_t>(&tensorflow_threads)->default_value(0), "amount of threads used by TensorFlow (0 = all available)")
#endif
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
#ifdef USE_TENSORFLOW_CC
                   .add(tf_inferencing_options)
#endif
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
        input_uri = variables["input"].as<std::string>();
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

    if (buffered_processing_frames < buffered_input_frames)
    {
        if (!quiet)
        {
            std::cout << "The internal packet buffer size must contain at least as many entries as the input buffer. Correct parameter." << std::endl;
        }

        buffered_processing_frames = buffered_input_frames;
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

        if (variables.count("tfthreads"))
        {
            tensorflow_threads = variables["tfthreads"].as<std::uint32_t>();

            if (tensorflow_threads == 0)
            {
                tensorflow_threads = std::thread::hardware_concurrency();
            }
        }
    }

    // create a single-threaded world, where the TensorFlow processor will live inside
    auto tf_scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                            boost::asynchronous::single_thread_scheduler<
                                boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> > >(std::string("tfpredict_processor"));

    std::shared_ptr<cvpg::imageproc::algorithms::tfpredict_processor_proxy> tfpredict_processor;

    if (variables.count("tfmodel"))
    {
        tfpredict_processor = std::make_shared<cvpg::imageproc::algorithms::tfpredict_processor_proxy>(tf_scheduler, tensorflow_threads);

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

    // determine if input is a video file or a video stream
    enum class input_mode_types
    {
        undefined,
        video,
        stream
    };

    input_mode_types input_mode = input_mode_types::undefined;

    // check if input is a video
    {
        std::regex rx(".*\\.mp4$");

        if (std::regex_match(input_uri, rx))
        {
            input_mode = input_mode_types::video;
        }
    }

    // check if input is a RTSP stream
    {
        std::regex rx("(rtsp?):\\/\\/(?:([^\\s@\\/]+?)[@])?([^\\s\\/:]+)(?:[:]([0-9]+))?(?:(\\/[^\\s?#]+)([?][^\\s#]+)?)?([#]\\S*)?");

        if (std::regex_match(input_uri, rx))
        {
            input_mode = input_mode_types::stream;
        }
    }

    if (!quiet)
    {
        std::cout << "Start processing ";

        switch (input_mode)
        {
            case input_mode_types::undefined:
                std::cout << "undefined input";
                break;

            case input_mode_types::video:
                std::cout << "video file";
                break;

            case input_mode_types::stream:
                std::cout << "RTSP stream";
                break;
        }

        std::cout << " from '" << input_uri << "'" << std::endl;
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

    // create a thread pool
    auto thread_pool = boost::asynchronous::make_shared_scheduler_proxy<
                           boost::asynchronous::multiqueue_threadpool_scheduler<
                               boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> > >(threads, std::string("threadpool"));

    if (!quiet)
    {
        std::cout << "Using threadpool with " << threads << " worker threads" << std::endl;
    }

    // create the image processor
    auto image_processor_scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                                         boost::asynchronous::single_thread_scheduler<
                                             boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> > >(std::string("image_processor"));

    cvpg::imageproc::scripting::image_processor_proxy image_processor(image_processor_scheduler, thread_pool);

#ifdef USE_TENSORFLOW_CC
    if (tfpredict_processor)
    {
        image_processor.add_param("tfmodel_processor", tfpredict_processor);
    }
#endif

    // create formatter to produce diagnostics in markdown format
    auto formatter_scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                                   boost::asynchronous::single_thread_scheduler<
                                       boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> > >(std::string("formatter"));

    using formatter_type = cvpg::imageproc::scripting::diagnostics::markdown_formatter<>;

    boost::asynchronous::formatter_proxy<formatter_type> diagnostics_formatter(formatter_scheduler,
                                                                               thread_pool,
                                                                               boost::asynchronous::make_scheduler_interfaces(/*image_processor_scheduler*/formatter_scheduler, thread_pool, formatter_scheduler));

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

    // create a scheduler for the source stage
    auto source_stage_scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                                      boost::asynchronous::single_thread_scheduler<
                                          boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> > >();

    // create a frame and interframe processor
    auto processors_scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                                    boost::asynchronous::single_thread_scheduler<
                                        boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> > >(std::string("processors"));

    cvpg::videoproc::any_stage<cvpg::image_rgb_8bit> frame_processor = std::make_shared<cvpg::videoproc::processors::image_rgb_8bit_frame_proxy>(processors_scheduler, buffered_processing_frames, image_processor);
    cvpg::videoproc::any_stage<cvpg::image_rgb_8bit> interframe_processor = std::make_shared<cvpg::videoproc::processors::image_rgb_8bit_interframe_proxy>(processors_scheduler, buffered_processing_frames, image_processor);

    // create a video file producer
    auto file_out_scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                                  boost::asynchronous::single_thread_scheduler<
                                      boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> > >();

    cvpg::videoproc::any_stage<cvpg::image_rgb_8bit> file_producer = std::make_shared<cvpg::videoproc::sinks::image_rgb_8bit_file_proxy>(file_out_scheduler, buffered_output_frames);

    // create a progress monitor
    auto progress_monitor_scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                                          boost::asynchronous::single_thread_scheduler<
                                              boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> > >();

    auto progress_monitor = std::make_shared<progress_monitor_proxy>(progress_monitor_scheduler, !quiet);

    // create a pipeline for the stages
    auto pipeline_scheduler = boost::asynchronous::make_shared_scheduler_proxy<
                                  boost::asynchronous::single_thread_scheduler<
                                      boost::asynchronous::lockfree_queue<cvpg::imageproc::scripting::diagnostics::servant_job> > >();

    auto promise_pipeline = std::make_shared<std::promise<std::string> >();

    cvpg::videoproc::any_stage<cvpg::image_rgb_8bit> source_stage;

    cvpg::videoproc::pipelines::any_pipeline pipeline;

    switch (input_mode)
    {
        case input_mode_types::undefined:
            break;

        case input_mode_types::video:
        {
            source_stage = std::make_shared<cvpg::videoproc::sources::image_rgb_8bit_file_proxy>(source_stage_scheduler, buffered_input_frames);
            pipeline = std::make_shared<cvpg::videoproc::pipelines::image_rgb_8bit_file_to_file_proxy>(pipeline_scheduler, source_stage, frame_processor, interframe_processor, file_producer);
            break;
        }

        case input_mode_types::stream:
        {
            source_stage = std::make_shared<cvpg::videoproc::sources::image_rgb_8bit_rtsp_proxy>(source_stage_scheduler, buffered_input_frames);
            pipeline = std::make_shared<cvpg::videoproc::pipelines::image_rgb_8bit_rtsp_to_file_proxy>(pipeline_scheduler, source_stage, frame_processor, interframe_processor, file_producer);
            break;
        }
    }

    (*pipeline).start(
        // stage parameters
        {
            input_uri,              // source stage
            frame_script,           // frame stage
            interframe_script,      // interframe stage
            output_filename         // sink stage
        },
        // callbacks
        {
            [promise_pipeline]()
            {
                promise_pipeline->set_value("");
            },
            [progress_monitor](std::size_t context_id, std::int64_t frames)
            {
                progress_monitor->init(context_id, frames);
            },
            [promise_pipeline](std::size_t /*context_id*/, std::string error)
            {
                promise_pipeline->set_value(std::move(error));
            },
            [progress_monitor](std::size_t context_id, cvpg::videoproc::update_indicator update) mutable
            {
                progress_monitor->update(context_id, std::move(update));
            }
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
