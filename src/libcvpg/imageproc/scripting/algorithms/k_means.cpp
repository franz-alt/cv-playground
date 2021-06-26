#include <libcvpg/imageproc/scripting/algorithms/k_means.hpp>

#include <chrono>
#include <functional>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/exception.hpp>
#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/algorithms/k_means.hpp>
#include <libcvpg/imageproc/scripting/item.hpp>
#include <libcvpg/imageproc/scripting/processing_context.hpp>
#include <libcvpg/imageproc/scripting/detail/compiler.hpp>
#include <libcvpg/imageproc/scripting/detail/handler.hpp>
#include <libcvpg/imageproc/scripting/detail/parser.hpp>

namespace detail {

struct k_means_task :  public boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >
{
    k_means_task(std::shared_ptr<cvpg::imageproc::scripting::processing_context> context, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
        : boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >("algorithms::k_means_task")
        , m_context(context)
        , m_result_id(result_id)
        , m_item(std::move(item))
    {}

    void operator()()
    {
        try
        {
            auto id = std::any_cast<std::uint32_t>(m_item.arguments.at(0).value());
            auto k = std::any_cast<std::int32_t>(m_item.arguments.at(1).value());
            auto max_iterations = std::any_cast<std::int32_t>(m_item.arguments.at(2).value());
            auto eps = std::any_cast<std::int32_t>(m_item.arguments.at(3).value());

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
                    cvpg::imageproc::algorithms::k_means(std::move(image), k, max_iterations, eps)
                );
            }
            else if (input.type() == cvpg::imageproc::scripting::item::types::rgb_8_bit_image)
            {
                auto image = std::any_cast<cvpg::image_rgb_8bit>(input.value());

                auto start = std::chrono::system_clock::now();

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
                    cvpg::imageproc::algorithms::k_means(std::move(image), k, max_iterations, eps)
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

auto k_means(std::shared_ptr<cvpg::imageproc::scripting::processing_context> context, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
{
    return boost::asynchronous::top_level_callback_continuation<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >(
               k_means_task(context, result_id, std::move(item))
           );
}

} // namespace detail

namespace cvpg::imageproc::scripting::algorithms {

std::string k_means::name() const
{
    return "k_means";
}

std::string k_means::category() const
{
    return "filters/segmentation";
}

std::vector<scripting::item::types> k_means::result() const
{
    return
    {
        scripting::item::types::grayscale_8_bit_image
    };
}

parameter_set k_means::parameters() const
{
    return parameter_set
           ({
               parameter("image", "input image", "", { scripting::item::types::grayscale_8_bit_image, scripting::item::types::rgb_8_bit_image }),
               parameter("k", "amount of clusters", "", scripting::item::types::signed_integer, static_cast<std::int32_t>(1), static_cast<std::int32_t>(255), static_cast<std::int32_t>(1)),
               parameter("max_iterations", "maximum amount of iterations", "", scripting::item::types::signed_integer, static_cast<std::int32_t>(1), static_cast<std::int32_t>(std::numeric_limits<std::int32_t>::max()), static_cast<std::int32_t>(1)),
               parameter("eps", "minimum distance in color space to mark two values as equal", "", scripting::item::types::signed_integer, static_cast<std::int32_t>(1), static_cast<std::int32_t>(441), static_cast<std::int32_t>(1))
           });
}

void k_means::on_parse(std::shared_ptr<detail::parser> parser) const
{
    // all parameters
    {
        std::function<std::uint32_t(std::uint32_t, std::int32_t, std::int32_t, std::int32_t)> fct =
            [parser, parameters = this->parameters()](std::uint32_t image_id, std::int32_t k, std::int32_t max_iterations, std::int32_t eps)
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

                if (!parameters.is_valid("k", k))
                {
                    throw cvpg::invalid_parameter_exception("invalid amount of clusters");
                }

                if (!parameters.is_valid("max_iterations", max_iterations))
                {
                    throw cvpg::invalid_parameter_exception("invalid amount of maximum iterations");
                }

                if (!parameters.is_valid("eps", eps))
                {
                    throw cvpg::invalid_parameter_exception("invalid epsilon");
                }

                std::uint32_t result_id = 0;

                switch (input_type)
                {
                    case scripting::item::types::grayscale_8_bit_image:
                    {
                        detail::parser::item result_item
                        {
                            "k_means",
                            {
                                scripting::item(scripting::item::types::grayscale_8_bit_image, image_id),
                                scripting::item(scripting::item::types::signed_integer, k),
                                scripting::item(scripting::item::types::signed_integer, max_iterations),
                                scripting::item(scripting::item::types::signed_integer, eps)
                            }
                        };

                        result_id = parser->register_item(std::move(result_item));

                        break;
                    }

                    case scripting::item::types::rgb_8_bit_image:
                    {
                        detail::parser::item result_item
                        {
                            "k_means",
                            {
                                scripting::item(scripting::item::types::rgb_8_bit_image, image_id),
                                scripting::item(scripting::item::types::signed_integer, k),
                                scripting::item(scripting::item::types::signed_integer, max_iterations),
                                scripting::item(scripting::item::types::signed_integer, eps)
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

    // default for 'eps' (minimum distance of two values in color space)
    {
        std::function<std::uint32_t(std::uint32_t, std::int32_t, std::int32_t)> fct =
            [parser, parameters = this->parameters()](std::uint32_t image_id, std::int32_t k, std::int32_t max_iterations)
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

                if (!parameters.is_valid("k", k))
                {
                    throw cvpg::invalid_parameter_exception("invalid amount of clusters");
                }

                if (!parameters.is_valid("max_iterations", max_iterations))
                {
                    throw cvpg::invalid_parameter_exception("invalid amount of maximum iterations");
                }

                std::uint32_t result_id = 0;

                const std::int32_t eps = 5;

                switch (input_type)
                {
                    case scripting::item::types::grayscale_8_bit_image:
                    {
                        detail::parser::item result_item
                        {
                            "k_means",
                            {
                                scripting::item(scripting::item::types::grayscale_8_bit_image, image_id),
                                scripting::item(scripting::item::types::signed_integer, k),
                                scripting::item(scripting::item::types::signed_integer, max_iterations),
                                scripting::item(scripting::item::types::signed_integer, eps)
                            }
                        };

                        result_id = parser->register_item(std::move(result_item));

                        break;
                    }

                    case scripting::item::types::rgb_8_bit_image:
                    {
                        detail::parser::item result_item
                        {
                            "k_means",
                            {
                                scripting::item(scripting::item::types::rgb_8_bit_image, image_id),
                                scripting::item(scripting::item::types::signed_integer, k),
                                scripting::item(scripting::item::types::signed_integer, max_iterations),
                                scripting::item(scripting::item::types::signed_integer, eps)
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

    // default for 'max_iterations'
    {
        std::function<std::uint32_t(std::uint32_t, std::int32_t)> fct =
            [parser, parameters = this->parameters()](std::uint32_t image_id, std::int32_t k)
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

                if (!parameters.is_valid("k", k))
                {
                    throw cvpg::invalid_parameter_exception("invalid amount of clusters");
                }

                std::uint32_t result_id = 0;

                const std::int32_t max_iterations = 10;
                const std::int32_t eps = 5;

                switch (input_type)
                {
                    case scripting::item::types::grayscale_8_bit_image:
                    {
                        detail::parser::item result_item
                        {
                            "k_means",
                            {
                                scripting::item(scripting::item::types::grayscale_8_bit_image, image_id),
                                scripting::item(scripting::item::types::signed_integer, k),
                                scripting::item(scripting::item::types::signed_integer, max_iterations),
                                scripting::item(scripting::item::types::signed_integer, eps)
                            }
                        };

                        result_id = parser->register_item(std::move(result_item));

                        break;
                    }

                    case scripting::item::types::rgb_8_bit_image:
                    {
                        detail::parser::item result_item
                        {
                            "k_means",
                            {
                                scripting::item(scripting::item::types::rgb_8_bit_image, image_id),
                                scripting::item(scripting::item::types::signed_integer, k),
                                scripting::item(scripting::item::types::signed_integer, max_iterations),
                                scripting::item(scripting::item::types::signed_integer, eps)
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

void k_means::on_compile(std::uint32_t item_id, std::shared_ptr<detail::compiler> compiler) const
{
    auto handler =
        detail::handler(
            [result_id = item_id, item = compiler->get_item(item_id)](std::shared_ptr<processing_context> context)
            {
                return ::detail::k_means(context, result_id, std::move(item));
            });

    compiler->register_handler(item_id, name(), std::move(handler));
}

} // namespace cvpg::imageproc::scripting::algorithms
