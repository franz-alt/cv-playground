// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/imageproc/scripting/algorithms/histogram_equalization.hpp>

#include <chrono>
#include <functional>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/exception.hpp>
#include <libcvpg/imageproc/algorithms/histogram_equalization.hpp>
#include <libcvpg/imageproc/scripting/item.hpp>
#include <libcvpg/imageproc/scripting/processing_context.hpp>
#include <libcvpg/imageproc/scripting/detail/compiler.hpp>
#include <libcvpg/imageproc/scripting/detail/handler.hpp>
#include <libcvpg/imageproc/scripting/detail/parser.hpp>

namespace detail {

struct histogram_equalization_task :  public boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >
{
    histogram_equalization_task(std::shared_ptr<cvpg::imageproc::scripting::processing_context> context, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
        : boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >("algorithms::histogram_equalization_task")
        , m_context(context)
        , m_result_id(result_id)
        , m_item(std::move(item))
    {}

    void operator()()
    {
        try
        {
            auto id = std::any_cast<std::uint32_t>(m_item.arguments.at(0).value());

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
                    cvpg::imageproc::algorithms::histogram_equalization(std::move(image))
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

auto histogram_equalization(std::shared_ptr<cvpg::imageproc::scripting::processing_context> context, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
{
    return boost::asynchronous::top_level_callback_continuation<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >(
               histogram_equalization_task(context, result_id, std::move(item))
           );
}

} // namespace detail

namespace cvpg { namespace imageproc { namespace scripting { namespace algorithms {

std::string histogram_equalization::name() const
{
    return "histogram_equalization";
}

std::string histogram_equalization::category() const
{
    return "filters/enhancement";
}

std::vector<scripting::item::types> histogram_equalization::result() const
{
    return
    {
        scripting::item::types::grayscale_8_bit_image
    };
}

parameter_set histogram_equalization::parameters() const
{
    return parameter_set
           (
               parameter("image", "input image", "", scripting::item::types::grayscale_8_bit_image)
           );
}

void histogram_equalization::on_parse(std::shared_ptr<detail::parser> parser) const
{
    std::function<std::uint32_t(std::uint32_t)> fct =
        [parser](std::uint32_t image_id)
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
            if (input_type != scripting::item::types::grayscale_8_bit_image)
            {
                throw cvpg::invalid_parameter_exception("invalid input type");
            }

            std::uint32_t result_id = 0;

            detail::parser::item result_item
            {
                "histogram_equalization",
                {
                    scripting::item(scripting::item::types::grayscale_8_bit_image, image_id)
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

void histogram_equalization::on_compile(std::uint32_t item_id, std::shared_ptr<detail::compiler> compiler) const
{
    auto handler =
        detail::handler(
            [result_id = item_id, item = compiler->get_item(item_id)](std::shared_ptr<processing_context> context)
            {
                return ::detail::histogram_equalization(context, result_id, std::move(item));
            });

    compiler->register_handler(item_id, name(), std::move(handler));
}

}}}} // namespace cvpg::imageproc::scripting::algorithms
