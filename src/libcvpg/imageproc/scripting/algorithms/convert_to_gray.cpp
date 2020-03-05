#include <libcvpg/imageproc/scripting/algorithms/convert_to_gray.hpp>

#include <chrono>
#include <functional>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/algorithms/convert_to_gray.hpp>
#include <libcvpg/imageproc/scripting/image_processor.hpp>
#include <libcvpg/imageproc/scripting/item.hpp>
#include <libcvpg/imageproc/scripting/detail/compiler.hpp>
#include <libcvpg/imageproc/scripting/detail/handler.hpp>
#include <libcvpg/imageproc/scripting/detail/parser.hpp>

namespace detail {

struct convert_to_gray_task :  public boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::image_processor> >
{
    convert_to_gray_task(std::shared_ptr<cvpg::imageproc::scripting::image_processor> image_processor, std::size_t context_id, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
        : boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::image_processor> >("algorithms::convert_to_gray_task")
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
            auto mode_str = std::any_cast<std::string>(m_item.arguments.at(1).value());

            auto input = m_image_processor->load(m_context_id, id);

            cvpg::imageproc::algorithms::rgb_conversion_mode mode;

            if (mode_str == "use_red")
            {
                mode = cvpg::imageproc::algorithms::rgb_conversion_mode::use_red;
            }
            else if (mode_str == "use_green")
            {
                mode = cvpg::imageproc::algorithms::rgb_conversion_mode::use_green;
            }
            else if (mode_str == "use_blue")
            {
                mode = cvpg::imageproc::algorithms::rgb_conversion_mode::use_blue;
            }
            else if (mode_str == "calc_average")
            {
                mode = cvpg::imageproc::algorithms::rgb_conversion_mode::calc_average;
            }
            else
            {
                // TODO error handling
            }

            if (input.type() == cvpg::imageproc::scripting::item::types::rgb_8_bit_image)
            {
                auto image = std::any_cast<cvpg::image_rgb_8bit>(input.value());

                auto start = std::chrono::system_clock::now();

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
                    cvpg::imageproc::algorithms::convert_to_gray(std::move(image), mode)
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

auto convert_to_gray(std::shared_ptr<cvpg::imageproc::scripting::image_processor> image_processor, std::size_t context_id, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
{
    return boost::asynchronous::top_level_callback_continuation<std::shared_ptr<cvpg::imageproc::scripting::image_processor> >(
               convert_to_gray_task(image_processor, context_id, result_id, std::move(item))
           );
}

} // namespace detail

namespace cvpg { namespace imageproc { namespace scripting { namespace algorithms {

std::string convert_to_gray::name() const
{
    return "convert_to_gray";
}

std::string convert_to_gray::category() const
{
    return "conversion";
}

std::vector<parameter::item::item_type> convert_to_gray::result() const
{
    return
    {
        parameter::item::item_type::grayscale_8_bit_image
    };
}

std::vector<std::vector<parameter> > convert_to_gray::parameters() const
{
    return std::vector<std::vector<parameter> >(
    {
        {
            parameter("image", "input image", "", parameter::item::item_type::rgb_8_bit_image),
            parameter("mode", "conversion mode", "", parameter::item::item_type::characters)
        }
    });
}

std::vector<std::string> convert_to_gray::check_parameters(std::vector<std::any> parameters) const
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

void convert_to_gray::on_parse(std::shared_ptr<detail::parser> parser) const
{
    std::function<std::uint32_t(std::uint32_t, std::string)> fct =
        [parser](std::uint32_t image_id, std::string mode)
        {
            if (mode != "use_red" || mode != "use_green" || mode != "use_blue" || mode != "calc_average")
            {
                // TODO error handling
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
                                "convert_to_gray",
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
                                "convert_to_gray",
                                {
                                    scripting::item(scripting::item::types::grayscale_8_bit_image, image_id),
                                    scripting::item(scripting::item::types::characters, mode)
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

void convert_to_gray::on_compile(std::uint32_t item_id, std::shared_ptr<detail::compiler> compiler) const
{
    auto handler =
        detail::handler(
            [result_id = item_id, item = compiler->get_item(item_id)](std::shared_ptr<image_processor> image_processor, std::size_t context_id)
            {
                return ::detail::convert_to_gray(image_processor, context_id, result_id, std::move(item));
            });

    compiler->register_handler(item_id, name(), std::move(handler));
}

}}}} // namespace cvpg::imageproc::scripting::algorithms
