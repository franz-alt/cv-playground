#include <libcvpg/imageproc/scripting/algorithms/paint_meta.hpp>

#include <chrono>
#include <functional>
#include <regex>
#include <string>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/exception.hpp>
#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/algorithms/paint_meta.hpp>
#include <libcvpg/imageproc/scripting/item.hpp>
#include <libcvpg/imageproc/scripting/processing_context.hpp>
#include <libcvpg/imageproc/scripting/detail/compiler.hpp>
#include <libcvpg/imageproc/scripting/detail/handler.hpp>
#include <libcvpg/imageproc/scripting/detail/parser.hpp>

namespace detail {

struct paint_meta_task :  public boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >
{
    paint_meta_task(std::shared_ptr<cvpg::imageproc::scripting::processing_context> context, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
        : boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >("algorithms::paint_meta_task")
        , m_context(context)
        , m_result_id(result_id)
        , m_item(std::move(item))
    {}

    void operator()()
    {
        try
        {
            auto id = std::any_cast<std::uint32_t>(m_item.arguments.at(0).value());
            auto key_str = std::any_cast<std::string>(m_item.arguments.at(1).value());
            auto mode_str = std::any_cast<std::string>(m_item.arguments.at(2).value());
            auto classes_key_str = std::any_cast<std::string>(m_item.arguments.at(3).value());
            auto classes_list_str = std::any_cast<std::string>(m_item.arguments.at(4).value());
            auto scores_key_str = std::any_cast<std::string>(m_item.arguments.at(5).value());
            auto min_score = std::any_cast<std::int32_t>(m_item.arguments.at(6).value());

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
                // extract color values from arguments
                auto gray = static_cast<std::uint8_t>(std::any_cast<std::int32_t>(m_item.arguments.at(7).value()));

                auto image = std::any_cast<cvpg::image_gray_8bit>(std::move(input.value()));

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
                    cvpg::imageproc::algorithms::paint_meta(std::move(image),
                                                            std::move(key_str),
                                                            std::move(mode_str),
                                                            std::move(classes_key_str),
                                                            std::move(classes_list_str),
                                                            std::move(scores_key_str),
                                                            min_score,
                                                            gray)
                );
            }
            else if (input.type() == cvpg::imageproc::scripting::item::types::rgb_8_bit_image)
            {
                // extract color values from arguments
                auto red = static_cast<std::uint8_t>(std::any_cast<std::int32_t>(m_item.arguments.at(7).value()));
                auto green = static_cast<std::uint8_t>(std::any_cast<std::int32_t>(m_item.arguments.at(8).value()));
                auto blue = static_cast<std::uint8_t>(std::any_cast<std::int32_t>(m_item.arguments.at(9).value()));

                auto image = std::any_cast<cvpg::image_rgb_8bit>(std::move(input.value()));

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
                    cvpg::imageproc::algorithms::paint_meta(std::move(image),
                                                            std::move(key_str),
                                                            std::move(mode_str),
                                                            std::move(classes_key_str),
                                                            std::move(classes_list_str),
                                                            std::move(scores_key_str),
                                                            min_score,
                                                            red,
                                                            green,
                                                            blue)
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

auto paint_meta(std::shared_ptr<cvpg::imageproc::scripting::processing_context> context, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
{
    return boost::asynchronous::top_level_callback_continuation<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >(
               paint_meta_task(context, result_id, std::move(item))
           );
}

} // namespace detail

namespace cvpg::imageproc::scripting::algorithms {

std::string paint_meta::name() const
{
    return "paint_meta";
}

std::string paint_meta::category() const
{
    return "utilities";
}

std::vector<scripting::item::types> paint_meta::result() const
{
    return
    {
        scripting::item::types::grayscale_8_bit_image,
        scripting::item::types::rgb_8_bit_image
    };
}

parameter_set paint_meta::parameters() const
{
    using namespace std::string_literals;

    return parameter_set
           ({
               parameter("image", "input image", "", { scripting::item::types::grayscale_8_bit_image, scripting::item::types::rgb_8_bit_image }),
               parameter("key", "key of metadata that should be painted", "", scripting::item::types::characters),
               parameter("mode", "paint mode", "", scripting::item::types::characters, "rectangle"s),
               parameter("color", "hexdecimal color code", "", scripting::item::types::characters),
               parameter("classes_key", "key of metadata that represents the detected classes", "", scripting::item::types::characters),
               parameter("classes_list", "comma separated list of classes that should be painted", "", scripting::item::types::characters),
               parameter("scores_key", "key of metadata that represents the scores of the detected classes", "", scripting::item::types::characters),
               parameter("min_score", "minimum score that will be painted", "percentage", scripting::item::types::signed_integer, static_cast<std::int32_t>(0), static_cast<std::int32_t>(99), static_cast<std::int32_t>(1))
           });
}

void paint_meta::on_parse(std::shared_ptr<detail::parser> parser) const
{
    std::function<std::uint32_t(std::uint32_t, std::string, std::string, std::string, std::string, std::string, std::string, std::uint32_t)> fct =
        [parser, parameters = this->parameters()](std::uint32_t image_id, std::string key, std::string mode, std::string color, std::string classes_key, std::string classes_list, std::string scores_key, std::uint32_t min_score)
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
                throw cvpg::invalid_parameter_exception("invalid operation mode");
            }

            if (!parameters.is_valid("min_score", static_cast<std::int32_t>(min_score)))
            {
                throw cvpg::invalid_parameter_exception("invalid minimum score");
            }

            std::uint32_t result_id = 0;

            switch (input_type)
            {
                case scripting::item::types::grayscale_8_bit_image:
                {
                    // check if given color string matches the correct regular expression
                    const std::regex base_regex("^\\#(([a-fA-F]|[0-9]){2})$");
                    std::smatch base_match;

                    if (!std::regex_match(color, base_match, base_regex))
                    {
                        throw cvpg::invalid_parameter_exception("invalid grayscale color");
                    }

                    // extract color values from color string
                    const std::uint8_t gray = std::stol (color.substr(1, 2), nullptr, 16);;

                    detail::parser::item result_item
                    {
                        "paint_meta",
                        {
                            scripting::item(scripting::item::types::grayscale_8_bit_image, image_id),
                            scripting::item(scripting::item::types::characters, key),
                            scripting::item(scripting::item::types::characters, mode),
                            scripting::item(scripting::item::types::characters, classes_key),
                            scripting::item(scripting::item::types::characters, classes_list),
                            scripting::item(scripting::item::types::characters, scores_key),
                            scripting::item(scripting::item::types::signed_integer, static_cast<std::int32_t>(min_score)),
                            scripting::item(scripting::item::types::signed_integer, static_cast<std::int32_t>(gray))
                        }
                    };

                    result_id = parser->register_item(std::move(result_item));

                    break;
                }

                case scripting::item::types::rgb_8_bit_image:
                {
                    // check if given color string matches the correct regular expression
                    const std::regex base_regex("^\\#(([a-fA-F]|[0-9]){6})$");
                    std::smatch base_match;

                    if (!std::regex_match(color, base_match, base_regex))
                    {
                        throw cvpg::invalid_parameter_exception("invalid RGB color");
                    }

                    // extract color values from color string
                    const std::uint8_t red = std::stol (color.substr(1, 2), nullptr, 16);;
                    const std::uint8_t green = std::stol (color.substr(3, 2), nullptr, 16);;
                    const std::uint8_t blue = std::stol (color.substr(5, 2), nullptr, 16);;

                    detail::parser::item result_item
                    {
                        "paint_meta",
                        {
                            scripting::item(scripting::item::types::rgb_8_bit_image, image_id),
                            scripting::item(scripting::item::types::characters, key),
                            scripting::item(scripting::item::types::characters, mode),
                            scripting::item(scripting::item::types::characters, classes_key),
                            scripting::item(scripting::item::types::characters, classes_list),
                            scripting::item(scripting::item::types::characters, scores_key),
                            scripting::item(scripting::item::types::signed_integer, static_cast<std::int32_t>(min_score)),
                            scripting::item(scripting::item::types::signed_integer, static_cast<std::int32_t>(red)),
                            scripting::item(scripting::item::types::signed_integer,static_cast<std::int32_t>( green)),
                            scripting::item(scripting::item::types::signed_integer, static_cast<std::int32_t>(blue)),
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

void paint_meta::on_compile(std::uint32_t item_id, std::shared_ptr<detail::compiler> compiler) const
{
    auto handler =
        detail::handler(
            [result_id = item_id, item = compiler->get_item(item_id)](std::shared_ptr<processing_context> context)
            {
                return ::detail::paint_meta(context, result_id, std::move(item));
            });

    compiler->register_handler(item_id, name(), std::move(handler));
}

} // namespace cvpg::imageproc::scripting::algorithms
