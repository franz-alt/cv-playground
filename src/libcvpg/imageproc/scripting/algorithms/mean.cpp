#include <libcvpg/imageproc/scripting/algorithms/mean.hpp>

#include <chrono>
#include <functional>
#include <string>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/exception.hpp>
#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/algorithms/tiling.hpp>
#include <libcvpg/imageproc/algorithms/tiling/mean.hpp>
#include <libcvpg/imageproc/scripting/image_processor.hpp>
#include <libcvpg/imageproc/scripting/item.hpp>
#include <libcvpg/imageproc/scripting/detail/compiler.hpp>
#include <libcvpg/imageproc/scripting/detail/handler.hpp>
#include <libcvpg/imageproc/scripting/detail/parser.hpp>

namespace detail {

struct mean_task :  public boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::image_processor> >
{
    mean_task(std::shared_ptr<cvpg::imageproc::scripting::image_processor> image_processor, std::size_t context_id, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
        : boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::image_processor> >("algorithms::mean_task")
        , m_image_processor(image_processor)
        , m_context_id(context_id)
        , m_result_id(result_id)
        , m_item(std::move(item))
    {}

    void operator()()
    {
        try
        {
            auto id = std::any_cast<std::uint32_t>(m_item.arguments.at(0).value());
            auto width = std::any_cast<std::uint32_t>(m_item.arguments.at(1).value());
            auto height = std::any_cast<std::uint32_t>(m_item.arguments.at(2).value());
            auto border_mode_str = std::any_cast<std::string>(m_item.arguments.at(3).value());

            auto input = m_image_processor->load(m_context_id, id);
            auto parameters = m_image_processor->parameters();

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

            auto border_mode = cvpg::imageproc::algorithms::to_border_mode(border_mode_str);

            if (input.type() == cvpg::imageproc::scripting::item::types::grayscale_8_bit_image)
            {
                auto image = std::any_cast<cvpg::image_gray_8bit>(input.value());

                const auto image_width = image.width();
                const auto image_height = image.height();

                auto start = std::chrono::system_clock::now();

                auto tf = cvpg::imageproc::algorithms::tiling_functors::image<cvpg::image_gray_8bit>({{ std::move(image) }});
                tf.parameters.image_width = image_width;
                tf.parameters.image_height = image_height;
                tf.parameters.cutoff_x = cutoff_x;
                tf.parameters.cutoff_y = cutoff_y;
                tf.parameters.signed_integer_numbers.push_back(width); // filter width
                tf.parameters.signed_integer_numbers.push_back(height); // filter height
                tf.parameters.border_mode = border_mode;

                tf.tile_algorithm_task = [](std::shared_ptr<cvpg::image_gray_8bit> src1, std::shared_ptr<cvpg::image_gray_8bit> /*src2*/, std::shared_ptr<cvpg::image_gray_8bit> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
                {
                    cvpg::imageproc::algorithms::mean_gray_8bit(src1->data(0).get(), dst->data(0).get(), from_x, to_x, from_y, to_y, std::move(parameters));
                };

                boost::asynchronous::create_callback_continuation(
                    [result = this->this_task_result(), image_processor = m_image_processor, result_id = m_result_id, context_id = m_context_id, start](auto cont_res) mutable
                    {
                        auto stop = std::chrono::system_clock::now();

                        try
                        {
                            image_processor->store(context_id, result_id, std::get<0>(cont_res).get(), std::chrono::duration_cast<std::chrono::microseconds>(stop - start));

                            result.set_value(image_processor);
                        }
                        catch (...)
                        {
                            result.set_exception(std::current_exception());
                        }
                    },
                    cvpg::imageproc::algorithms::tiling(std::move(tf))
                );
            }
            else if (input.type() == cvpg::imageproc::scripting::item::types::rgb_8_bit_image)
            {
                auto image = std::any_cast<cvpg::image_rgb_8bit>(input.value());

                const auto image_width = image.width();
                const auto image_height = image.height();

                auto start = std::chrono::system_clock::now();

                auto tf = cvpg::imageproc::algorithms::tiling_functors::image<cvpg::image_rgb_8bit>({{ std::move(image) }});
                tf.parameters.image_width = image_width;
                tf.parameters.image_height = image_height;
                tf.parameters.cutoff_x = cutoff_x;
                tf.parameters.cutoff_y = cutoff_y;
                tf.parameters.signed_integer_numbers.push_back(width); // filter width
                tf.parameters.signed_integer_numbers.push_back(height); // filter height
                tf.parameters.border_mode = border_mode;

                tf.tile_algorithm_task = [](std::shared_ptr<cvpg::image_rgb_8bit> src1, std::shared_ptr<cvpg::image_rgb_8bit> /*src2*/, std::shared_ptr<cvpg::image_rgb_8bit> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
                {
                    cvpg::imageproc::algorithms::mean_gray_8bit(src1->data(0).get(), dst->data(0).get(), from_x, to_x, from_y, to_y, parameters);
                    cvpg::imageproc::algorithms::mean_gray_8bit(src1->data(1).get(), dst->data(1).get(), from_x, to_x, from_y, to_y, parameters);
                    cvpg::imageproc::algorithms::mean_gray_8bit(src1->data(2).get(), dst->data(2).get(), from_x, to_x, from_y, to_y, std::move(parameters));
                };

                boost::asynchronous::create_callback_continuation(
                    [result = this->this_task_result(), image_processor = m_image_processor, result_id = m_result_id, context_id = m_context_id, start](auto cont_res) mutable
                    {
                        auto stop = std::chrono::system_clock::now();

                        try
                        {
                            image_processor->store(context_id, result_id, std::get<0>(cont_res).get(), std::chrono::duration_cast<std::chrono::microseconds>(stop - start));

                            result.set_value(image_processor);
                        }
                        catch (...)
                        {
                            result.set_exception(std::current_exception());
                        }
                    },
                    cvpg::imageproc::algorithms::tiling(std::move(tf))
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
    std::shared_ptr<cvpg::imageproc::scripting::image_processor> m_image_processor;

    std::size_t m_context_id;

    std::uint32_t m_result_id;

    cvpg::imageproc::scripting::detail::parser::item m_item;
};

auto mean(std::shared_ptr<cvpg::imageproc::scripting::image_processor> image_processor, std::size_t context_id, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
{
    return boost::asynchronous::top_level_callback_continuation<std::shared_ptr<cvpg::imageproc::scripting::image_processor> >(
               mean_task(image_processor, context_id, result_id, std::move(item))
           );
}

} // namespace detail

namespace cvpg { namespace imageproc { namespace scripting { namespace algorithms {

std::string mean::name() const
{
    return "mean";
}

std::string mean::category() const
{
    return "filters/smoothing";
}

std::vector<parameter::item::item_type> mean::result() const
{
    return
    {
        parameter::item::item_type::grayscale_8_bit_image,
        parameter::item::item_type::rgb_8_bit_image,
    };
}

parameter_set mean::parameters() const
{
    using namespace std::string_literals;

    return parameter_set
           ({
               parameter("image", "input image", "", { parameter::item::item_type::grayscale_8_bit_image, parameter::item::item_type::rgb_8_bit_image }),
               parameter("filter_width", "filter width", "pixels", parameter::item::item_type::unsigned_integer, static_cast<std::uint32_t>(3), static_cast<std::uint32_t>(65535), static_cast<std::uint32_t>(2)),
               parameter("filter_height", "filter height", "pixels", parameter::item::item_type::unsigned_integer, static_cast<std::uint32_t>(3), static_cast<std::uint32_t>(65535), static_cast<std::uint32_t>(2)),
               parameter("border_mode", "border mode", "", parameter::item::item_type::characters, { "ignore"s, "constant"s, "mirror"s })
           });
}

void mean::on_parse(std::shared_ptr<detail::parser> parser) const
{
    // all parameters
    {
        std::function<std::uint32_t(std::uint32_t, std::uint32_t, std::uint32_t, std::string)> fct =
            [parser, parameters = this->parameters()](std::uint32_t image_id, std::uint32_t width, std::uint32_t height, std::string border_mode)
            {
                // check parameters
                // if (!parameters.is_valid("image", image_id)
                // {
                //     throw cvpg::invalid_parameter_exception("invalid input mode");
                // }

                if (!parameters.is_valid("filter_width", width))
                {
                    throw cvpg::invalid_parameter_exception("invalid filter width");
                }

                if (!parameters.is_valid("filter_height", height))
                {
                    throw cvpg::invalid_parameter_exception("invalid filter height");
                }

                if (!parameters.is_valid("border_mode", border_mode))
                {
                    throw cvpg::invalid_parameter_exception("invalid border mode");
                }

                std::uint32_t result_id = 0;

                // find image
                if (!!parser)
                {
                    auto image = parser->find_item(image_id);

                    if (image.arguments.size() != 0 && image.arguments.front().type() != scripting::item::types::invalid)
                    {
                        switch (image.arguments.front().type())
                        {
                            case scripting::item::types::grayscale_8_bit_image:
                            {
                                detail::parser::item result_item
                                {
                                    "mean",
                                    {
                                        scripting::item(scripting::item::types::grayscale_8_bit_image, image_id),
                                        scripting::item(scripting::item::types::unsigned_integer, width),
                                        scripting::item(scripting::item::types::unsigned_integer, height),
                                        scripting::item(scripting::item::types::characters, border_mode)
                                    }
                                };

                                result_id = parser->register_item(std::move(result_item));

                                break;
                            }

                            case scripting::item::types::rgb_8_bit_image:
                            {
                                detail::parser::item result_item
                                {
                                    "mean",
                                    {
                                        scripting::item(scripting::item::types::rgb_8_bit_image, image_id),
                                        scripting::item(scripting::item::types::unsigned_integer, width),
                                        scripting::item(scripting::item::types::unsigned_integer, height),
                                        scripting::item(scripting::item::types::characters, border_mode)
                                    }
                                };

                                result_id = parser->register_item(std::move(result_item));

                                break;
                            }

                            default:
                            {
                                // TODO error handling

                                break;
                            }
                        }

                        if (result_id != 0)
                        {
                            parser->register_link(image_id, result_id);
                        }
                    }
                }

                return result_id;
            };

        parser->register_specification(name(), std::move(fct));
    }

    // default for border mode
    {
        std::function<std::uint32_t(std::uint32_t, std::uint32_t, std::uint32_t)> fct =
            [parser, parameters = this->parameters()](std::uint32_t image_id, std::uint32_t width, std::uint32_t height)
            {
                // check parameters
                // if (!parameters.is_valid("image", image_id)
                // {
                //     throw cvpg::invalid_parameter_exception("invalid input mode");
                // }

                if (!parameters.is_valid("filter_width", width))
                {
                    throw cvpg::invalid_parameter_exception("invalid filter width");
                }

                if (!parameters.is_valid("filter_height", width))
                {
                    throw cvpg::invalid_parameter_exception("invalid filter height");
                }

                std::uint32_t result_id = 0;
                std::string border_mode = "constant";

                // find image
                if (!!parser)
                {
                    auto image = parser->find_item(image_id);

                    if (image.arguments.size() != 0 && image.arguments.front().type() != scripting::item::types::invalid)
                    {
                        switch (image.arguments.front().type())
                        {
                            case scripting::item::types::grayscale_8_bit_image:
                            {
                                detail::parser::item result_item
                                {
                                    "mean",
                                    {
                                        scripting::item(scripting::item::types::grayscale_8_bit_image, image_id),
                                        scripting::item(scripting::item::types::unsigned_integer, width),
                                        scripting::item(scripting::item::types::unsigned_integer, height),
                                        scripting::item(scripting::item::types::characters, border_mode)
                                    }
                                };

                                result_id = parser->register_item(std::move(result_item));

                                break;
                            }

                            case scripting::item::types::rgb_8_bit_image:
                            {
                                detail::parser::item result_item
                                {
                                    "mean",
                                    {
                                        scripting::item(scripting::item::types::rgb_8_bit_image, image_id),
                                        scripting::item(scripting::item::types::unsigned_integer, width),
                                        scripting::item(scripting::item::types::unsigned_integer, height),
                                        scripting::item(scripting::item::types::characters, border_mode)
                                    }
                                };

                                result_id = parser->register_item(std::move(result_item));

                                break;
                            }

                            default:
                            {
                                // TODO error handling

                                break;
                            }
                        }

                        if (result_id != 0)
                        {
                            parser->register_link(image_id, result_id);
                        }
                    }
                }

                return result_id;
            };

        parser->register_specification(name(), std::move(fct));
    }
}

void mean::on_compile(std::uint32_t item_id, std::shared_ptr<detail::compiler> compiler) const
{
    auto handler =
        detail::handler(
            [result_id = item_id, item = compiler->get_item(item_id)](std::shared_ptr<image_processor> image_processor, std::size_t context_id)
            {
                return ::detail::mean(image_processor, context_id, result_id, std::move(item));
            });

    compiler->register_handler(item_id, name(), std::move(handler));
}

}}}} // namespace cvpg::imageproc::scripting::algorithms
