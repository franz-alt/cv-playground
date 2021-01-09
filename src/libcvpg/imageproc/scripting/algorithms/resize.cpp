#include <libcvpg/imageproc/scripting/algorithms/resize.hpp>

#include <chrono>
#include <functional>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/exception.hpp>
#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/algorithms/tiling.hpp>
#include <libcvpg/imageproc/algorithms/tiling/resize.hpp>
#include <libcvpg/imageproc/scripting/item.hpp>
#include <libcvpg/imageproc/scripting/processing_context.hpp>
#include <libcvpg/imageproc/scripting/detail/compiler.hpp>
#include <libcvpg/imageproc/scripting/detail/handler.hpp>
#include <libcvpg/imageproc/scripting/detail/parser.hpp>

namespace detail {

struct resize_task :  public boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >
{
    resize_task(std::shared_ptr<cvpg::imageproc::scripting::processing_context> context, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
        : boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >("algorithms::resize_task")
        , m_context(context)
        , m_result_id(result_id)
        , m_item(std::move(item))
    {}

    void operator()()
    {
        try
        {
            auto id = std::any_cast<std::uint32_t>(m_item.arguments.at(0).value());

            std::int32_t width = 0;
            std::int32_t height = 0;

            // determine width and height
            if (m_item.arguments.size() == 3)
            {
                // width and height from parameters
                width = std::any_cast<std::int32_t>(m_item.arguments.at(1).value());
                height = std::any_cast<std::int32_t>(m_item.arguments.at(2).value());
            }
            else
            {
                // width and height from destination image
                auto destination_id = std::any_cast<std::uint32_t>(m_item.arguments.at(1).value());

                auto destination = m_context->load(destination_id);

                if (destination.type() == cvpg::imageproc::scripting::item::types::grayscale_8_bit_image)
                {
                    auto destination_image = std::any_cast<cvpg::image_gray_8bit>(destination.value());

                    width = destination_image.width();
                    height = destination_image.height();
                }
                else if (destination.type() == cvpg::imageproc::scripting::item::types::rgb_8_bit_image)
                {
                    auto destination_image = std::any_cast<cvpg::image_rgb_8bit>(destination.value());

                    width = destination_image.width();
                    height = destination_image.height();
                }
                else
                {
                    throw cvpg::invalid_parameter_exception("invalid type of destination image");
                }
            }

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

                auto start = std::chrono::system_clock::now();

                auto tf = cvpg::imageproc::algorithms::tiling_functors::image<cvpg::image_gray_8bit>({{ std::move(image) }});
                tf.parameters.image_width = image.width();
                tf.parameters.image_height = image.height();
                tf.parameters.dst_image_width = width;
                tf.parameters.dst_image_height = height;
                tf.parameters.cutoff_x = cutoff_x;
                tf.parameters.cutoff_y = cutoff_y;

                tf.tile_algorithm_task = [](std::shared_ptr<cvpg::image_gray_8bit> src1, std::shared_ptr<cvpg::image_gray_8bit> /*src2*/, std::shared_ptr<cvpg::image_gray_8bit> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
                {
                    cvpg::imageproc::algorithms::resize_gray_8bit(src1->data(0).get(), dst->data(0).get(), from_x, to_x, from_y, to_y, std::move(parameters));
                };

                boost::asynchronous::create_callback_continuation(
                    [result = this->this_task_result(), context = m_context, result_id = m_result_id, start](auto cont_res) mutable
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
            }
            else if (input.type() == cvpg::imageproc::scripting::item::types::rgb_8_bit_image)
            {
                auto image = std::any_cast<cvpg::image_rgb_8bit>(input.value());

                auto start = std::chrono::system_clock::now();

                auto tf = cvpg::imageproc::algorithms::tiling_functors::image<cvpg::image_rgb_8bit>({{ std::move(image) }});
                tf.parameters.image_width = image.width();
                tf.parameters.image_height = image.height();
                tf.parameters.dst_image_width = width;
                tf.parameters.dst_image_height = height;
                tf.parameters.cutoff_x = cutoff_x;
                tf.parameters.cutoff_y = cutoff_y;

                tf.tile_algorithm_task = [](std::shared_ptr<cvpg::image_rgb_8bit> src1, std::shared_ptr<cvpg::image_rgb_8bit> /*src2*/, std::shared_ptr<cvpg::image_rgb_8bit> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
                {
                    cvpg::imageproc::algorithms::resize_gray_8bit(src1->data(0).get(), dst->data(0).get(), from_x, to_x, from_y, to_y, parameters);
                    cvpg::imageproc::algorithms::resize_gray_8bit(src1->data(1).get(), dst->data(1).get(), from_x, to_x, from_y, to_y, parameters);
                    cvpg::imageproc::algorithms::resize_gray_8bit(src1->data(2).get(), dst->data(2).get(), from_x, to_x, from_y, to_y, std::move(parameters));
                };

                boost::asynchronous::create_callback_continuation(
                    [result = this->this_task_result(), context = m_context, result_id = m_result_id, start](auto cont_res) mutable
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

auto resize(std::shared_ptr<cvpg::imageproc::scripting::processing_context> context, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
{
    return boost::asynchronous::top_level_callback_continuation<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >(
               resize_task(context, result_id, std::move(item))
           );
}

} // namespace detail

namespace cvpg::imageproc::scripting::algorithms {

std::string resize::name() const
{
    return "resize";
}

std::string resize::category() const
{
    return "filters/geometric_transformation";
}

std::vector<scripting::item::types> resize::result() const
{
    return
    {
        scripting::item::types::grayscale_8_bit_image,
        scripting::item::types::rgb_8_bit_image
    };
}

parameter_set resize::parameters() const
{
    return parameter_set
           ({
               parameter("image", "input image", "", { scripting::item::types::grayscale_8_bit_image, scripting::item::types::rgb_8_bit_image }),
               parameter("width", "width", "pixels", scripting::item::types::signed_integer, static_cast<std::int32_t>(1), static_cast<std::int32_t>(2147483647), static_cast<std::int32_t>(1)),
               parameter("height", "height", "pixels", scripting::item::types::signed_integer, static_cast<std::int32_t>(1), static_cast<std::int32_t>(2147483647), static_cast<std::int32_t>(1))
           });
}

void resize::on_parse(std::shared_ptr<detail::parser> parser) const
{
    std::function<std::uint32_t(std::uint32_t, std::int32_t, std::int32_t)> fct =
        [parser, parameters = this->parameters()](std::uint32_t image_id, std::int32_t width, std::int32_t height)
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

            if (!parameters.is_valid("width", width))
            {
                throw cvpg::invalid_parameter_exception("invalid new image width");
            }

            if (!parameters.is_valid("height", height))
            {
                throw cvpg::invalid_parameter_exception("invalid new image height");
            }

            std::uint32_t result_id = 0;

            switch (input_type)
            {
                case scripting::item::types::grayscale_8_bit_image:
                {
                    detail::parser::item result_item
                    {
                        "resize",
                        {
                            scripting::item(scripting::item::types::grayscale_8_bit_image, image_id),
                            scripting::item(scripting::item::types::signed_integer, width),
                            scripting::item(scripting::item::types::signed_integer, height)
                        }
                    };

                    result_id = parser->register_item(std::move(result_item));

                    break;
                }

                case scripting::item::types::rgb_8_bit_image:
                {
                    detail::parser::item result_item
                    {
                        "resize",
                        {
                            scripting::item(scripting::item::types::rgb_8_bit_image, image_id),
                            scripting::item(scripting::item::types::signed_integer, width),
                            scripting::item(scripting::item::types::signed_integer, height)
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

void resize::on_compile(std::uint32_t item_id, std::shared_ptr<detail::compiler> compiler) const
{
    auto handler =
        detail::handler(
            [result_id = item_id, item = compiler->get_item(item_id)](std::shared_ptr<processing_context> context)
            {
                return ::detail::resize(context, result_id, std::move(item));
            });

    compiler->register_handler(item_id, name(), std::move(handler));
}

// ------------------------------------------------------------------------------------------------

std::string resize_to::name() const
{
    return "resize_to";
}

std::string resize_to::category() const
{
    return "filters/geometric_transformation";
}

std::vector<scripting::item::types> resize_to::result() const
{
    return
    {
        scripting::item::types::grayscale_8_bit_image,
        scripting::item::types::rgb_8_bit_image
    };
}

parameter_set resize_to::parameters() const
{
    return parameter_set
           ({
               parameter("image", "input image", "", { scripting::item::types::grayscale_8_bit_image, scripting::item::types::rgb_8_bit_image }),
               parameter("destination", "destination image", "", { scripting::item::types::grayscale_8_bit_image, scripting::item::types::rgb_8_bit_image })
           });
}

void resize_to::on_parse(std::shared_ptr<detail::parser> parser) const
{
    std::function<std::uint32_t(std::uint32_t, std::uint32_t)> fct =
        [parser, parameters = this->parameters()](std::uint32_t image_id, std::uint32_t destination_id)
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

            auto destination = parser->find_item(destination_id);

            if (destination.arguments.empty())
            {
                throw cvpg::invalid_parameter_exception("invalid destination ID");
            }

            auto destination_type = destination.arguments.front().type();

            // check parameters
            if (!(destination_type == scripting::item::types::grayscale_8_bit_image || destination_type == scripting::item::types::rgb_8_bit_image))
            {
                throw cvpg::invalid_parameter_exception("invalid destination");
            }

            std::uint32_t result_id = 0;

            switch (input_type)
            {
                case scripting::item::types::grayscale_8_bit_image:
                {
                    detail::parser::item result_item
                    {
                        "resize_to",
                        {
                            scripting::item(scripting::item::types::grayscale_8_bit_image, image_id),
                            scripting::item(scripting::item::types::grayscale_8_bit_image, destination_id)
                        }
                    };

                    result_id = parser->register_item(std::move(result_item));

                    break;
                }

                case scripting::item::types::rgb_8_bit_image:
                {
                    detail::parser::item result_item
                    {
                        "resize_to",
                        {
                            scripting::item(scripting::item::types::rgb_8_bit_image, image_id),
                            scripting::item(scripting::item::types::rgb_8_bit_image, destination_id)
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

void resize_to::on_compile(std::uint32_t item_id, std::shared_ptr<detail::compiler> compiler) const
{
    auto handler =
        detail::handler(
            [result_id = item_id, item = compiler->get_item(item_id)](std::shared_ptr<processing_context> context)
            {
                return ::detail::resize(context, result_id, std::move(item));
            });

    compiler->register_handler(item_id, name(), std::move(handler));
}

} // namespace cvpg::imageproc::scripting::algorithms
