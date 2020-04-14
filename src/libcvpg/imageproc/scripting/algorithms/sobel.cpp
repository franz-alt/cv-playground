#include <libcvpg/imageproc/scripting/algorithms/sobel.hpp>

#include <chrono>
#include <functional>
#include <string>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/algorithms/border_mode.hpp>
#include <libcvpg/imageproc/algorithms/tiling.hpp>
#include <libcvpg/imageproc/algorithms/tiling/sobel.hpp>
#include <libcvpg/imageproc/scripting/image_processor.hpp>
#include <libcvpg/imageproc/scripting/item.hpp>
#include <libcvpg/imageproc/scripting/detail/compiler.hpp>
#include <libcvpg/imageproc/scripting/detail/handler.hpp>
#include <libcvpg/imageproc/scripting/detail/parser.hpp>

namespace detail {

struct sobel_task :  public boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::image_processor> >
{
    sobel_task(std::shared_ptr<cvpg::imageproc::scripting::image_processor> image_processor, std::size_t context_id, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
        : boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::image_processor> >("algorithms::sobel_task")
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
            auto size = std::any_cast<std::int32_t>(m_item.arguments.at(1).value());
            auto mode_str = std::any_cast<std::string>(m_item.arguments.at(2).value());
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

            cvpg::imageproc::algorithms::sobel_operation_mode mode;

            if (mode_str == "hor")
            {
                mode = cvpg::imageproc::algorithms::sobel_operation_mode::horizontal;
            }
            else if (mode_str == "vert")
            {
                mode = cvpg::imageproc::algorithms::sobel_operation_mode::vertical;
            }
            else
            {
                // TODO error handling
            }

            auto border_mode = cvpg::imageproc::algorithms::to_border_mode(border_mode_str);

            if (input.type() == cvpg::imageproc::scripting::item::types::grayscale_8_bit_image)
            {
                auto image = std::any_cast<cvpg::image_gray_8bit>(input.value());

                const auto width = image.width();
                const auto height = image.height();

                auto start = std::chrono::system_clock::now();

                auto tf = cvpg::imageproc::algorithms::tiling_functors::image<cvpg::image_gray_8bit>({{ std::move(image) }});
                tf.parameters.image_width = width;
                tf.parameters.image_height = height;
                tf.parameters.cutoff_x = cutoff_x;
                tf.parameters.cutoff_y = cutoff_y;
                tf.parameters.signed_integer_numbers.push_back(size);
                tf.parameters.border_mode = border_mode;

                tf.tile_algorithm_task = [mode](std::shared_ptr<cvpg::image_gray_8bit> src1, std::shared_ptr<cvpg::image_gray_8bit> /*src2*/, std::shared_ptr<cvpg::image_gray_8bit> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
                {
                    cvpg::imageproc::algorithms::sobel_gray_8bit(src1->data(0).get(), dst->data(0).get(), from_x, to_x, from_y, to_y, std::move(parameters), mode);
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

                const auto width = image.width();
                const auto height = image.height();

                auto start = std::chrono::system_clock::now();

                auto tf = cvpg::imageproc::algorithms::tiling_functors::image<cvpg::image_rgb_8bit>({{ std::move(image) }});
                tf.parameters.image_width = width;
                tf.parameters.image_height = height;
                tf.parameters.cutoff_x = cutoff_x;
                tf.parameters.cutoff_y = cutoff_y;
                tf.parameters.signed_integer_numbers.push_back(size);
                tf.parameters.border_mode = border_mode;

                tf.tile_algorithm_task = [mode](std::shared_ptr<cvpg::image_rgb_8bit> src1, std::shared_ptr<cvpg::image_rgb_8bit> /*src2*/, std::shared_ptr<cvpg::image_rgb_8bit> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
                {
                    cvpg::imageproc::algorithms::sobel_gray_8bit(src1->data(0).get(), dst->data(0).get(), from_x, to_x, from_y, to_y, parameters, mode);
                    cvpg::imageproc::algorithms::sobel_gray_8bit(src1->data(1).get(), dst->data(1).get(), from_x, to_x, from_y, to_y, parameters, mode);
                    cvpg::imageproc::algorithms::sobel_gray_8bit(src1->data(2).get(), dst->data(2).get(), from_x, to_x, from_y, to_y, std::move(parameters), mode);
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

auto sobel(std::shared_ptr<cvpg::imageproc::scripting::image_processor> image_processor, std::size_t context_id, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
{
    return boost::asynchronous::top_level_callback_continuation<std::shared_ptr<cvpg::imageproc::scripting::image_processor> >(
               sobel_task(image_processor, context_id, result_id, std::move(item))
           );
}

} // namespace detail

namespace cvpg { namespace imageproc { namespace scripting { namespace algorithms {

std::string sobel::name() const
{
    return "sobel";
}

std::string sobel::category() const
{
    return "filters/edge";
}

std::vector<parameter::item::item_type> sobel::result() const
{
    return
    {
        parameter::item::item_type::grayscale_8_bit_image,
        parameter::item::item_type::rgb_8_bit_image
    };
}

std::vector<std::vector<parameter> > sobel::parameters() const
{
    return std::vector<std::vector<parameter> >(
    {
        {
            parameter("image", "input image", "", parameter::item::item_type::grayscale_8_bit_image),
            parameter("size", "size of filter mask", "", parameter::item::item_type::signed_integer, [min_value = 3, max_value = 5](std::any element)
                                                                                                     {
                                                                                                         std::int16_t value = std::any_cast<std::int16_t>(element);

                                                                                                         return (value >= min_value && value <= max_value) && (value % 2 == 1);
                                                                                                     }),
            parameter("mode", "operation mode", "", parameter::item::item_type::characters),
            parameter("border_mode", "border mode", "", parameter::item::item_type::characters)
        },
        {
            parameter("image", "input image", "", parameter::item::item_type::rgb_8_bit_image),
            parameter("size", "size of filter mask", "", parameter::item::item_type::signed_integer, [min_value = 3, max_value = 5](std::any element)
                                                                                                     {
                                                                                                         std::int16_t value = std::any_cast<std::int16_t>(element);

                                                                                                         return (value >= min_value && value <= max_value) && (value % 2 == 1);
                                                                                                     }),
            parameter("mode", "operation mode", "", parameter::item::item_type::characters),
            parameter("border_mode", "border mode", "", parameter::item::item_type::characters)
        }
    });
}

std::vector<std::string> sobel::check_parameters(std::vector<std::any> parameters) const
{
    std::vector<std::string> messages;

    if (parameters.size() != this->parameters().size())
    {
        messages.emplace_back(std::string("Invalid amount of parameters. Expecting ").append(std::to_string(this->parameters().size())).append(" parameters."));
        return messages;
    }

    // TODO check image parameter

    return messages;
}

void sobel::on_parse(std::shared_ptr<detail::parser> parser) const
{
    // all parameters
    {
        std::function<std::uint32_t(std::uint32_t, std::int32_t, std::string, std::string)> fct =
        [parser](std::uint32_t image_id, std::int32_t size, std::string mode, std::string border_mode)
        {
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
                                "sobel",
                                {
                                    scripting::item(scripting::item::types::grayscale_8_bit_image, image_id),
                                    scripting::item(scripting::item::types::signed_integer, size),
                                    scripting::item(scripting::item::types::characters, mode),
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
                                "sobel",
                                {
                                    scripting::item(scripting::item::types::rgb_8_bit_image, image_id),
                                    scripting::item(scripting::item::types::signed_integer, size),
                                    scripting::item(scripting::item::types::characters, mode),
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
                else
                {
                    // TODO error handling
                }
            }

            return result_id;
        };

        parser->register_specification(name(), std::move(fct));
    }

    // default for border mode
    {
        std::function<std::uint32_t(std::uint32_t, std::int32_t, std::string)> fct =
            [parser](std::uint32_t image_id, std::int32_t size, std::string mode)
            {
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
                                    "sobel",
                                    {
                                        scripting::item(scripting::item::types::grayscale_8_bit_image, image_id),
                                        scripting::item(scripting::item::types::signed_integer, size),
                                        scripting::item(scripting::item::types::characters, mode),
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
                                    "sobel",
                                    {
                                        scripting::item(scripting::item::types::rgb_8_bit_image, image_id),
                                        scripting::item(scripting::item::types::signed_integer, size),
                                        scripting::item(scripting::item::types::characters, mode),
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
                    else
                    {
                        // TODO error handling
                    }
                }

                return result_id;
            };

        parser->register_specification(name(), std::move(fct));
    }
}

void sobel::on_compile(std::uint32_t item_id, std::shared_ptr<detail::compiler> compiler) const
{
    auto handler =
        detail::handler(
            [result_id = item_id, item = compiler->get_item(item_id)](std::shared_ptr<image_processor> image_processor, std::size_t context_id)
            {
                return ::detail::sobel(image_processor, context_id, result_id, std::move(item));
            });

    compiler->register_handler(item_id, name(), std::move(handler));
}

}}}} // namespace cvpg::imageproc::scripting::algorithms
