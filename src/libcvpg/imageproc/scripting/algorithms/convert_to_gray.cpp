// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/imageproc/scripting/algorithms/convert_to_gray.hpp>

#include <chrono>
#include <functional>
#include <string>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/exception.hpp>
#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/algorithms/convert_to_gray.hpp>
#include <libcvpg/imageproc/scripting/item.hpp>
#include <libcvpg/imageproc/scripting/processing_context.hpp>
#include <libcvpg/imageproc/scripting/detail/compiler.hpp>
#include <libcvpg/imageproc/scripting/detail/handler.hpp>
#include <libcvpg/imageproc/scripting/detail/parser.hpp>

namespace detail {

struct convert_to_gray_task :  public boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >
{
    convert_to_gray_task(std::shared_ptr<cvpg::imageproc::scripting::processing_context> context, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
        : boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >("algorithms::convert_to_gray_task")
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

            if (input.type() == cvpg::imageproc::scripting::item::types::rgb_8_bit_image)
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
                    cvpg::imageproc::algorithms::convert_to_gray(std::move(image), mode)
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

auto convert_to_gray(std::shared_ptr<cvpg::imageproc::scripting::processing_context> context, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
{
    return boost::asynchronous::top_level_callback_continuation<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >(
               convert_to_gray_task(context, result_id, std::move(item))
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

std::vector<scripting::item::types> convert_to_gray::result() const
{
    return
    {
        scripting::item::types::grayscale_8_bit_image
    };
}

parameter_set convert_to_gray::parameters() const
{
    using namespace std::string_literals;

    return parameter_set
           ({
               parameter("image", "input image", "", scripting::item::types::rgb_8_bit_image),
               parameter("mode", "conversion mode", "", scripting::item::types::characters, { "use_red"s, "use_green"s, "use_blue"s, "calc_average"s })
           });
}

void convert_to_gray::on_parse(std::shared_ptr<detail::parser> parser) const
{
    std::function<std::uint32_t(std::uint32_t, std::string)> fct =
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
            if (input_type != scripting::item::types::rgb_8_bit_image)
            {
                throw cvpg::invalid_parameter_exception("invalid input type");
            }

            if (!parameters.is_valid("mode", mode))
            {
                throw cvpg::invalid_parameter_exception("invalid conversion mode");
            }

            std::uint32_t result_id = 0;

            detail::parser::item result_item
            {
                "convert_to_gray",
                {
                    scripting::item(scripting::item::types::grayscale_8_bit_image, image_id),
                    scripting::item(scripting::item::types::characters, mode)
                }
            };

            result_id = parser->register_item(std::move(result_item));

            if (result_id != 0)
            {
                parser->register_link(image_id, result_id);
            }

            return result_id;
        };

    parser->register_specification(name(), std::move(fct));
}

void convert_to_gray::on_compile(std::uint32_t item_id, std::shared_ptr<detail::compiler> compiler) const
{
    auto handler =
        detail::handler(
            [result_id = item_id, item = compiler->get_item(item_id)](std::shared_ptr<processing_context> context)
            {
                return ::detail::convert_to_gray(context, result_id, std::move(item));
            });

    compiler->register_handler(item_id, name(), std::move(handler));
}

}}}} // namespace cvpg::imageproc::scripting::algorithms
