#include <libcvpg/imageproc/scripting/algorithms/binary_threshold.hpp>

#include <chrono>
#include <functional>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/exception.hpp>
#include <libcvpg/core/histogram.hpp>
#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/algorithms/otsu_threshold.hpp>
#include <libcvpg/imageproc/algorithms/tiling.hpp>
#include <libcvpg/imageproc/algorithms/tiling/histogram.hpp>
#include <libcvpg/imageproc/algorithms/tiling/threshold.hpp>
#include <libcvpg/imageproc/algorithms/tiling/functors/histogram.hpp>
#include <libcvpg/imageproc/scripting/item.hpp>
#include <libcvpg/imageproc/scripting/processing_context.hpp>
#include <libcvpg/imageproc/scripting/detail/compiler.hpp>
#include <libcvpg/imageproc/scripting/detail/handler.hpp>
#include <libcvpg/imageproc/scripting/detail/parser.hpp>

namespace detail {

struct merge_histograms_task : public boost::asynchronous::continuation_task<std::shared_ptr<cvpg::histogram<std::size_t> > >
{
    merge_histograms_task(std::shared_ptr<cvpg::histogram<std::size_t> > a, std::shared_ptr<cvpg::histogram<std::size_t> > b)
        : boost::asynchronous::continuation_task<std::shared_ptr<cvpg::histogram<std::size_t> > >("algorithms::binary_threshold_task::merge_histograms")
        , m_a(a)
        , m_b(b)
    {}

    void operator()()
    {
        // TODO optimize this
        this_task_result().set_value(std::make_shared<cvpg::histogram<std::size_t> >(*m_a + *m_b));
    }

private:
    std::shared_ptr<cvpg::histogram<std::size_t> > m_a;
    std::shared_ptr<cvpg::histogram<std::size_t> > m_b;
};

boost::asynchronous::detail::callback_continuation<std::shared_ptr<cvpg::histogram<std::size_t> > >
merge_histograms(std::shared_ptr<cvpg::histogram<std::size_t> > a, std::shared_ptr<cvpg::histogram<std::size_t> > b)
{
    return boost::asynchronous::top_level_callback_continuation<std::shared_ptr<cvpg::histogram<std::size_t> > >(
               merge_histograms_task(a, b)
           );
}

struct binary_threshold_task :  public boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >
{
    binary_threshold_task(std::shared_ptr<cvpg::imageproc::scripting::processing_context> context, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
        : boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >("algorithms::binary_threshold_task")
        , m_context(context)
        , m_result_id(result_id)
        , m_item(std::move(item))
    {}

    void operator()()
    {
        try
        {
            auto id = std::any_cast<std::uint32_t>(m_item.arguments.at(0).value());
            auto mode_str = std::any_cast<std::string>(m_item.arguments.at(1).value());

            auto input = m_context->load(id);
            auto parameters = m_context->parameters();

            std::uint32_t cutoff_x = 512;
            std::uint32_t cutoff_y = 512;

            {
                auto it = parameters.find("cutoff_x");

                if (it != parameters.end())
                {
                    cutoff_x = std::any_cast<std::uint32_t>(it->second);
                }
            }

            {
                auto it = parameters.find("cutoff_y");

                if (it != parameters.end())
                {
                    cutoff_y = std::any_cast<std::uint32_t>(it->second);
                }
            }

            if (input.type() == cvpg::imageproc::scripting::item::types::grayscale_8_bit_image)
            {
                auto image = std::any_cast<cvpg::image_gray_8bit>(input.value());

                const auto width = image.width();
                const auto height = image.height();

                auto start = std::chrono::system_clock::now();

                auto tf = cvpg::imageproc::algorithms::tiling_functors::histogram<cvpg::image_gray_8bit, cvpg::histogram<std::size_t> >({{ image }});
                tf.parameters.image_width = width;
                tf.parameters.image_height = height;
                tf.parameters.cutoff_x = cutoff_x;
                tf.parameters.cutoff_y = cutoff_y;

                tf.tile_algorithm_task = [](std::shared_ptr<cvpg::image_gray_8bit> src1, std::shared_ptr<cvpg::image_gray_8bit> /*src2*/, std::shared_ptr<cvpg::histogram<std::size_t> > dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
                {
                    std::vector<std::size_t> h(256);
                    cvpg::imageproc::algorithms::histogram_gray_8bit(src1->data(0).get(), &h, from_x, to_x, from_y, to_y, std::move(parameters));

                    *dst = cvpg::histogram<std::size_t>(std::move(h));
                };

                tf.horizontal_merge_task = [](std::shared_ptr<cvpg::histogram<std::size_t> > dst1, std::shared_ptr<cvpg::histogram<std::size_t> > dst2, std::size_t /*from_x*/, std::size_t /*to_x*/, std::size_t /*from_y*/, std::size_t /*to_y*/, cvpg::imageproc::algorithms::tiling_parameters /*parameters*/)
                {
                    return merge_histograms(dst1, dst2);
                };

                tf.vertical_merge_task = [](std::shared_ptr<cvpg::histogram<std::size_t> > dst1, std::shared_ptr<cvpg::histogram<std::size_t> > dst2, std::size_t /*from_x*/, std::size_t /*to_x*/, std::size_t /*from_y*/, std::size_t /*to_y*/, cvpg::imageproc::algorithms::tiling_parameters /*parameters*/)
                {
                    return merge_histograms(dst1, dst2);
                };

                boost::asynchronous::create_callback_continuation(
                    [result = this->this_task_result(), context = m_context, result_id = m_result_id, image = std::move(image), mode_str = std::move(mode_str), cutoff_x, cutoff_y, start = std::move(start)](auto cont_res) mutable
                    {
                        auto histogram = std::move(std::get<0>(cont_res).get());

                        const std::size_t threshold = cvpg::imageproc::algorithms::otsu_threshold(histogram);

                        auto tf = cvpg::imageproc::algorithms::tiling_functors::image<cvpg::image_gray_8bit>({{ std::move(image) }});
                        tf.parameters.image_width = image.width();
                        tf.parameters.image_height = image.height();
                        tf.parameters.cutoff_x = cutoff_x;
                        tf.parameters.cutoff_y = cutoff_y;
                        tf.parameters.signed_integer_numbers.push_back(threshold);

                        tf.tile_algorithm_task = [mode_str = std::move(mode_str)](std::shared_ptr<cvpg::image_gray_8bit> src1, std::shared_ptr<cvpg::image_gray_8bit> /*src2*/, std::shared_ptr<cvpg::image_gray_8bit> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters) mutable
                        {
                            if (mode_str == "normal")
                            {
                                cvpg::imageproc::algorithms::threshold_gray_8bit(src1->data(0).get(), dst->data(0).get(), from_x, to_x, from_y, to_y, std::move(parameters));
                            }
                            else if (mode_str == "inverse")
                            {
                                cvpg::imageproc::algorithms::threshold_inverse_gray_8bit(src1->data(0).get(), dst->data(0).get(), from_x, to_x, from_y, to_y, std::move(parameters));
                            }
                            else
                            {
                                // TODO error handling
                            }
                        };

                        boost::asynchronous::create_callback_continuation(
                            [result = std::move(result), context = std::move(context), result_id, start = std::move(start)](auto cont_res) mutable
                            {
                                auto stop = std::chrono::system_clock::now();

                                try
                                {
                                    context->store(result_id, std::get<0>(cont_res).get(), std::chrono::duration_cast<std::chrono::microseconds>(stop - start));

                                    result.set_value(context);
                                }
                                catch (...)
                                {
                                    result.set_exception(std::current_exception());
                                }
                            },
                            cvpg::imageproc::algorithms::tiling(std::move(tf))
                        );
                    },
                    cvpg::imageproc::algorithms::tiling(std::move(tf))
                );
            }
            else if (input.type() == cvpg::imageproc::scripting::item::types::rgb_8_bit_image)
            {
                auto image_rgb = std::any_cast<cvpg::image_rgb_8bit>(input.value());

                const auto image_width = image_rgb.width();
                const auto image_height = image_rgb.height();
                const auto image_padding = image_rgb.padding();

                // create separate images for each channel
                auto image_red = cvpg::image_gray_8bit(image_width, image_height, image_padding, { image_rgb.data(0) });
                auto image_green = cvpg::image_gray_8bit(image_width, image_height, image_padding, { image_rgb.data(1) });
                auto image_blue = cvpg::image_gray_8bit(image_width, image_height, image_padding, { image_rgb.data(2) });

                auto start = std::chrono::system_clock::now();

                auto create_channel_histogram =
                    [cutoff_x, cutoff_y](auto image)
                    {
                        auto tf = cvpg::imageproc::algorithms::tiling_functors::histogram<cvpg::image_gray_8bit, cvpg::histogram<std::size_t> >({{ image }});
                        tf.parameters.image_width = image.width();
                        tf.parameters.image_height = image.height();
                        tf.parameters.cutoff_x = cutoff_x;
                        tf.parameters.cutoff_y = cutoff_y;

                        tf.tile_algorithm_task = [](std::shared_ptr<cvpg::image_gray_8bit> src1, std::shared_ptr<cvpg::image_gray_8bit> /*src2*/, std::shared_ptr<cvpg::histogram<std::size_t> > dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
                        {
                            std::vector<std::size_t> h(256);
                            cvpg::imageproc::algorithms::histogram_gray_8bit(src1->data(0).get(), &h, from_x, to_x, from_y, to_y, std::move(parameters));

                            *dst = cvpg::histogram<std::size_t>(std::move(h));
                        };

                        tf.horizontal_merge_task = [](std::shared_ptr<cvpg::histogram<std::size_t> > dst1, std::shared_ptr<cvpg::histogram<std::size_t> > dst2, std::size_t /*from_x*/, std::size_t /*to_x*/, std::size_t /*from_y*/, std::size_t /*to_y*/, cvpg::imageproc::algorithms::tiling_parameters /*parameters*/)
                        {
                            return merge_histograms(dst1, dst2);
                        };

                        tf.vertical_merge_task = [](std::shared_ptr<cvpg::histogram<std::size_t> > dst1, std::shared_ptr<cvpg::histogram<std::size_t> > dst2, std::size_t /*from_x*/, std::size_t /*to_x*/, std::size_t /*from_y*/, std::size_t /*to_y*/, cvpg::imageproc::algorithms::tiling_parameters /*parameters*/)
                        {
                            return merge_histograms(dst1, dst2);
                        };

                        return tf;
                    };

                std::vector<boost::asynchronous::detail::callback_continuation<cvpg::histogram<std::size_t> > > tiling_functors;
                tiling_functors.reserve(3);
                tiling_functors.emplace_back(cvpg::imageproc::algorithms::tiling(create_channel_histogram(image_red)));
                tiling_functors.emplace_back(cvpg::imageproc::algorithms::tiling(create_channel_histogram(image_green)));
                tiling_functors.emplace_back(cvpg::imageproc::algorithms::tiling(create_channel_histogram(image_blue)));

                boost::asynchronous::create_callback_continuation(
                    [result = this->this_task_result()
                    ,context = m_context
                    ,result_id = m_result_id
                    ,image_width
                    ,image_height
                    ,image_padding
                    ,image_red = std::move(image_red)
                    ,image_green = std::move(image_green)
                    ,image_blue = std::move(image_blue)
                    ,mode_str = std::move(mode_str)
                    ,cutoff_x
                    ,cutoff_y
                    ,start = std::move(start)](auto cont_res) mutable
                    {
                        auto stop = std::chrono::system_clock::now();

                        try
                        {
                            const std::size_t threshold_red = cvpg::imageproc::algorithms::otsu_threshold(std::move(cont_res[0].get()));
                            const std::size_t threshold_green = cvpg::imageproc::algorithms::otsu_threshold(std::move(cont_res[1].get()));
                            const std::size_t threshold_blue = cvpg::imageproc::algorithms::otsu_threshold(std::move(cont_res[2].get()));

                            auto create_channel_threshold_image =
                                [cutoff_x, cutoff_y, mode_str](const auto & image, std::size_t threshold)
                                {
                                    auto tf = cvpg::imageproc::algorithms::tiling_functors::image<cvpg::image_gray_8bit>({{ std::move(image) }});
                                    tf.parameters.image_width = image.width();
                                    tf.parameters.image_height = image.height();
                                    tf.parameters.cutoff_x = cutoff_x;
                                    tf.parameters.cutoff_y = cutoff_y;
                                    tf.parameters.signed_integer_numbers.push_back(threshold);

                                    tf.tile_algorithm_task = [mode_str = std::move(mode_str)](std::shared_ptr<cvpg::image_gray_8bit> src1, std::shared_ptr<cvpg::image_gray_8bit> /*src2*/, std::shared_ptr<cvpg::image_gray_8bit> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters) mutable
                                    {
                                        if (mode_str == "normal")
                                        {
                                            cvpg::imageproc::algorithms::threshold_gray_8bit(src1->data(0).get(), dst->data(0).get(), from_x, to_x, from_y, to_y, std::move(parameters));
                                        }
                                        else if (mode_str == "inverse")
                                        {
                                            cvpg::imageproc::algorithms::threshold_inverse_gray_8bit(src1->data(0).get(), dst->data(0).get(), from_x, to_x, from_y, to_y, std::move(parameters));
                                        }
                                        else
                                        {
                                            // TODO error handling
                                        }
                                    };

                                    return tf;
                                };

                            std::vector<boost::asynchronous::detail::callback_continuation<cvpg::image_gray_8bit> > tiling_functors;
                            tiling_functors.reserve(3);
                            tiling_functors.emplace_back(cvpg::imageproc::algorithms::tiling(create_channel_threshold_image(image_red, threshold_red)));
                            tiling_functors.emplace_back(cvpg::imageproc::algorithms::tiling(create_channel_threshold_image(image_green, threshold_green)));
                            tiling_functors.emplace_back(cvpg::imageproc::algorithms::tiling(create_channel_threshold_image(image_blue, threshold_blue)));

                            boost::asynchronous::create_callback_continuation(
                                [result = std::move(result), context = std::move(context), image_width, image_height, image_padding, result_id, start = std::move(start)](auto cont_res) mutable
                                {
                                    auto stop = std::chrono::system_clock::now();

                                    try
                                    {
                                        context->store(result_id,
                                                       cvpg::image_rgb_8bit(
                                                           image_width,
                                                           image_height,
                                                           image_padding,
                                                           {
                                                               cont_res[0].get().data(0),
                                                               cont_res[1].get().data(0),
                                                               cont_res[2].get().data(0)
                                                           }
                                                       ),
                                                       std::chrono::duration_cast<std::chrono::microseconds>(stop - start));

                                        result.set_value(context);
                                    }
                                    catch (...)
                                    {
                                        result.set_exception(std::current_exception());
                                    }
                                },
                                std::move(tiling_functors)
                            );
                        }
                        catch (...)
                        {
                            result.set_exception(std::current_exception());
                        }
                    },
                    std::move(tiling_functors)
                );
            }
            else
            {
                // TODO error handling
            }
        }
        catch (...)
        {
            this->this_task_result().set_exception(std::current_exception());
        }
    }

private:
    std::shared_ptr<cvpg::imageproc::scripting::processing_context> m_context;

    std::uint32_t m_result_id;

    cvpg::imageproc::scripting::detail::parser::item m_item;
};

auto binary_threshold(std::shared_ptr<cvpg::imageproc::scripting::processing_context> context, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
{
    return boost::asynchronous::top_level_callback_continuation<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >(
               binary_threshold_task(context, result_id, std::move(item))
           );
}

} // namespace detail

namespace cvpg::imageproc::scripting::algorithms {

std::string binary_threshold::name() const
{
    return "binary_threshold";
}

std::string binary_threshold::category() const
{
    return "filters/segmentation";
}

std::vector<scripting::item::types> binary_threshold::result() const
{
    return
    {
        scripting::item::types::grayscale_8_bit_image,
        scripting::item::types::rgb_8_bit_image
    };
}

parameter_set binary_threshold::parameters() const
{
    using namespace std::string_literals;

    return parameter_set
           ({
               parameter("image", "input image", "", { scripting::item::types::grayscale_8_bit_image, scripting::item::types::rgb_8_bit_image }),
               parameter("mode", "conversion mode", "", scripting::item::types::characters, { "normal"s, "inverse"s })
           });
}

void binary_threshold::on_parse(std::shared_ptr<detail::parser> parser) const
{
    std::function<std::uint32_t(std::uint32_t, std::string mode)> fct =
        [parser, parameters = this->parameters()](std::uint32_t image_id, std::string mode)
        {
            // find image
            if (!parser)
            {
                throw cvpg::invalid_parameter_exception("invalid parser");
            }

            auto image = parser->find_item(image_id);

            if (image.arguments.empty())
            {
                throw cvpg::invalid_parameter_exception("invalid input ID");
            }

            auto input_type = image.arguments.front().type();

            // check parameters
            if (!(input_type == scripting::item::types::grayscale_8_bit_image || input_type == scripting::item::types::rgb_8_bit_image))
            {
                throw cvpg::invalid_parameter_exception("invalid input");
            }

            if (!parameters.is_valid("mode", mode))
            {
                throw cvpg::invalid_parameter_exception("invalid conversion mode");
            }

            std::uint32_t result_id = 0;

            switch (input_type)
            {
                case scripting::item::types::grayscale_8_bit_image:
                {
                    detail::parser::item result_item
                    {
                        "binary_threshold",
                        {
                            scripting::item(scripting::item::types::grayscale_8_bit_image, image_id),
                            scripting::item(scripting::item::types::characters, mode)
                        }
                    };

                    result_id = parser->register_item(std::move(result_item));

                    break;
                }

                case scripting::item::types::rgb_8_bit_image:
                {
                    detail::parser::item result_item
                    {
                        "binary_threshold",
                        {
                            scripting::item(scripting::item::types::rgb_8_bit_image, image_id),
                            scripting::item(scripting::item::types::characters, mode)
                        }
                    };

                    result_id = parser->register_item(std::move(result_item));

                    break;
                }

                default:
                {
                    // to make the compiler happy ; other input types are not allowed and should be handled above
                    break;
                }
            }

            if (result_id != 0)
            {
                parser->register_link(image_id, result_id);
            }

            return result_id;
        };

    parser->register_specification(name(), std::move(fct));
}

void binary_threshold::on_compile(std::uint32_t item_id, std::shared_ptr<detail::compiler> compiler) const
{
    auto handler =
        detail::handler(
            [result_id = item_id, item = compiler->get_item(item_id)](std::shared_ptr<processing_context> context)
            {
                return ::detail::binary_threshold(context, result_id, std::move(item));
            });

    compiler->register_handler(item_id, name(), std::move(handler));
}

} // namespace cvpg::imageproc::scripting::algorithms
