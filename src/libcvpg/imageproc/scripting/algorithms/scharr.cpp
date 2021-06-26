#include <libcvpg/imageproc/scripting/algorithms/scharr.hpp>

#include <chrono>
#include <functional>
#include <string>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/exception.hpp>
#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/algorithms/border_mode.hpp>
#include <libcvpg/imageproc/algorithms/tiling.hpp>
#include <libcvpg/imageproc/algorithms/tiling/scharr.hpp>
#include <libcvpg/imageproc/scripting/item.hpp>
#include <libcvpg/imageproc/scripting/processing_context.hpp>
#include <libcvpg/imageproc/scripting/detail/compiler.hpp>
#include <libcvpg/imageproc/scripting/detail/handler.hpp>
#include <libcvpg/imageproc/scripting/detail/parser.hpp>

namespace detail {

struct scharr_task :  public boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >
{
    scharr_task(std::shared_ptr<cvpg::imageproc::scripting::processing_context> context, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
        : boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >("algorithms::scharr_task")
        , m_context(context)
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

            cvpg::imageproc::algorithms::scharr_operation_mode mode;

            if (mode_str == "hor")
            {
                mode = cvpg::imageproc::algorithms::scharr_operation_mode::horizontal;
            }
            else if (mode_str == "vert")
            {
                mode = cvpg::imageproc::algorithms::scharr_operation_mode::vertical;
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
                    cvpg::imageproc::algorithms::scharr_gray_8bit(src1->data(0).get(), dst->data(0).get(), from_x, to_x, from_y, to_y, std::move(parameters), mode);
                };

                boost::asynchronous::create_callback_continuation(
                    [result = this->this_task_result(), context = m_context, result_id = m_result_id, start](auto cont_res) mutable
                    {
                        auto stop = std::chrono::system_clock::now();

                        try
                        {
                            context->store(result_id, std::move(std::get<0>(cont_res).get()), std::chrono::duration_cast<std::chrono::microseconds>(stop - start));

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
                    cvpg::imageproc::algorithms::scharr_gray_8bit(src1->data(0).get(), dst->data(0).get(), from_x, to_x, from_y, to_y, parameters, mode);
                    cvpg::imageproc::algorithms::scharr_gray_8bit(src1->data(1).get(), dst->data(1).get(), from_x, to_x, from_y, to_y, parameters, mode);
                    cvpg::imageproc::algorithms::scharr_gray_8bit(src1->data(2).get(), dst->data(2).get(), from_x, to_x, from_y, to_y, std::move(parameters), mode);
                };

                boost::asynchronous::create_callback_continuation(
                    [result = this->this_task_result(), context = m_context, result_id = m_result_id, start](auto cont_res) mutable
                    {
                        auto stop = std::chrono::system_clock::now();

                        try
                        {
                            context->store(result_id, std::move(std::get<0>(cont_res).get()), std::chrono::duration_cast<std::chrono::microseconds>(stop - start));

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

auto scharr(std::shared_ptr<cvpg::imageproc::scripting::processing_context> context, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
{
    return boost::asynchronous::top_level_callback_continuation<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >(
               scharr_task(context, result_id, std::move(item))
           );
}

} // namespace detail

namespace cvpg { namespace imageproc { namespace scripting { namespace algorithms {

std::string scharr::name() const
{
    return "scharr";
}

std::string scharr::category() const
{
    return "filters/edge";
}

std::vector<scripting::item::types> scharr::result() const
{
    return
    {
        scripting::item::types::grayscale_8_bit_image,
        scripting::item::types::rgb_8_bit_image
    };
}

parameter_set scharr::parameters() const
{
    using namespace std::string_literals;

    return parameter_set
           ({
               parameter("image", "input image", "", { scripting::item::types::grayscale_8_bit_image, scripting::item::types::rgb_8_bit_image }),
               parameter("size", "size of filter mask", "", scripting::item::types::signed_integer, static_cast<std::int32_t>(3)),
               parameter("mode", "operation mode", "", scripting::item::types::characters, { "hor"s, "vert"s }),
               parameter("border_mode", "border mode", "", scripting::item::types::characters, { "ignore"s, "constant"s, "mirror"s })
           });
}

void scharr::on_parse(std::shared_ptr<detail::parser> parser) const
{
    // all parameters
    {
        std::function<std::uint32_t(std::uint32_t, std::int32_t, std::string, std::string)> fct =
            [parser, parameters = this->parameters()](std::uint32_t image_id, std::int32_t size, std::string mode, std::string border_mode)
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
                    throw cvpg::invalid_parameter_exception("invalid input type");
                }

                if (!parameters.is_valid("size", size))
                {
                    throw cvpg::invalid_parameter_exception("invalid size of filter mask");
                }

                if (!parameters.is_valid("mode", mode))
                {
                    throw cvpg::invalid_parameter_exception("invalid operation mode");
                }

                if (!parameters.is_valid("border_mode", border_mode))
                {
                    throw cvpg::invalid_parameter_exception("invalid border mode");
                }

                std::uint32_t result_id = 0;

                switch (input_type)
                {
                    case scripting::item::types::grayscale_8_bit_image:
                    {
                        detail::parser::item result_item
                        {
                            "scharr",
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
                            "scharr",
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

    // default for border mode
    {
        std::function<std::uint32_t(std::uint32_t, std::int32_t, std::string)> fct =
            [parser, parameters = this->parameters()](std::uint32_t image_id, std::int32_t size, std::string mode)
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
                    throw cvpg::invalid_parameter_exception("invalid input type");
                }

                if (!parameters.is_valid("size", size))
                {
                    throw cvpg::invalid_parameter_exception("invalid size of filter mask");
                }

                if (!parameters.is_valid("mode", mode))
                {
                    throw cvpg::invalid_parameter_exception("invalid operation mode");
                }

                std::uint32_t result_id = 0;

                std::string border_mode = "constant";

                switch (input_type)
                {
                    case scripting::item::types::grayscale_8_bit_image:
                    {
                        detail::parser::item result_item
                        {
                            "scharr",
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
                            "scharr",
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
}

void scharr::on_compile(std::uint32_t item_id, std::shared_ptr<detail::compiler> compiler) const
{
    auto handler =
        detail::handler(
            [result_id = item_id, item = compiler->get_item(item_id)](std::shared_ptr<processing_context> context)
            {
                return ::detail::scharr(context, result_id, std::move(item));
            });

    compiler->register_handler(item_id, name(), std::move(handler));
}

}}}} // namespace cvpg::imageproc::scripting::algorithms
