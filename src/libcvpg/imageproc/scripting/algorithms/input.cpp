#include <libcvpg/imageproc/scripting/algorithms/input.hpp>

#include <chrono>
#include <functional>
#include <string>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/exception.hpp>
#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/algorithms/tiling.hpp>
#include <libcvpg/imageproc/scripting/image_processor.hpp>
#include <libcvpg/imageproc/scripting/item.hpp>
#include <libcvpg/imageproc/scripting/detail/compiler.hpp>
#include <libcvpg/imageproc/scripting/detail/handler.hpp>
#include <libcvpg/imageproc/scripting/detail/parser.hpp>

namespace detail {

struct input_task :  public boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::image_processor> >
{
    input_task(std::shared_ptr<cvpg::imageproc::scripting::image_processor> image_processor, std::size_t context_id, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
        : boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::image_processor> >("algorithms::input_task")
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

            auto input = m_image_processor->load(m_context_id, id);

            if (input.type() == cvpg::imageproc::scripting::item::types::grayscale_8_bit_image)
            {
                m_image_processor->store(m_context_id, m_result_id, std::move(std::any_cast<cvpg::image_gray_8bit>(input.value())));
            }
            else if (input.type() == cvpg::imageproc::scripting::item::types::rgb_8_bit_image)
            {
                m_image_processor->store(m_context_id, m_result_id, std::move(std::any_cast<cvpg::image_rgb_8bit>(input.value())));
            }
            else
            {
                // TODO error handling
            }

            this->this_task_result().set_value(m_image_processor);
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

auto input(std::shared_ptr<cvpg::imageproc::scripting::image_processor> image_processor, std::size_t context_id, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
{
    return boost::asynchronous::top_level_callback_continuation<std::shared_ptr<cvpg::imageproc::scripting::image_processor> >(
               input_task(image_processor, context_id, result_id, std::move(item))
           );
}

} // namespace detail

namespace cvpg { namespace imageproc { namespace scripting { namespace algorithms {

std::string input::name() const
{
    return "input";
}

std::string input::category() const
{
    return "input";
}

std::vector<scripting::item::types> input::result() const
{
    return
    {
        scripting::item::types::grayscale_8_bit_image,
        scripting::item::types::rgb_8_bit_image
    };
}

parameter_set input::parameters() const
{
    using namespace std::string_literals;

    return parameter_set
           ({
               parameter("mode", "type of input image", "", scripting::item::types::characters, { "gray"s, "rgb"s }),
               parameter("bits", "amount of bits", "", scripting::item::types::signed_integer, static_cast<std::int32_t>(8)),
               parameter("source", "ID of input source", "", scripting::item::types::signed_integer)
           });
}

void input::on_parse(std::shared_ptr<detail::parser> parser) const
{
    std::function<std::uint32_t(std::string, std::uint8_t)> fct1 =
        [parser, parameters = this->parameters()](std::string mode, std::uint8_t bits)
        {
            // check parameters
            if (!parameters.is_valid("mode", mode))
            {
                throw cvpg::invalid_parameter_exception("invalid input mode");
            }

            if (!parameters.is_valid("bits", static_cast<std::int32_t>(bits)))
            {
                throw cvpg::invalid_parameter_exception("unsupported bits per pixel");
            }

            std::uint32_t result_id = 0;

            if (mode == "gray")
            {
                if (bits == 8)
                {
                    detail::parser::item result_item
                    {
                        "input",
                        {
                            scripting::item(scripting::item::types::grayscale_8_bit_image, static_cast<std::uint32_t>(0)), // TODO 0 is at moment the ID of the input image
                            scripting::item(scripting::item::types::signed_integer, static_cast<std::int32_t>(8))
                        }
                    };

                    result_id = parser->register_item(std::move(result_item));
                }
            }
            else if (mode == "rgb")
            {
                if (bits == 8)
                {
                    detail::parser::item result_item
                    {
                        "input",
                        {
                            scripting::item(scripting::item::types::rgb_8_bit_image, static_cast<std::uint32_t>(0)), // TODO 0 is at moment the ID of the input image
                            scripting::item(scripting::item::types::signed_integer, static_cast<std::int32_t>(8))
                        }
                    };

                    result_id = parser->register_item(std::move(result_item));
                }
            }

            return result_id;
        };

    parser->register_specification(name(), std::move(fct1));

    std::function<std::uint32_t(std::string, std::uint8_t, std::uint8_t)> fct2 =
        [parser, parameters = this->parameters()](std::string mode, std::uint8_t bits, std::uint8_t source)
        {
            // check parameters
            if (!parameters.is_valid("mode", mode))
            {
                throw cvpg::invalid_parameter_exception("invalid input mode");
            }

            if (!parameters.is_valid("bits", static_cast<std::int32_t>(bits)))
            {
                throw cvpg::invalid_parameter_exception("unsupported bits per pixel");
            }

            std::uint32_t result_id = 0;

            if (mode == "gray")
            {
                if (bits == 8)
                {
                    detail::parser::item result_item
                    {
                        "input",
                        {
                            scripting::item(scripting::item::types::grayscale_8_bit_image, static_cast<std::uint32_t>(source) - 1),
                            scripting::item(scripting::item::types::signed_integer, static_cast<std::int32_t>(8))
                        }
                    };

                    result_id = parser->register_item(std::move(result_item));
                }
            }
            else if (mode == "rgb")
            {
                if (bits == 8)
                {
                    detail::parser::item result_item
                    {
                        "input",
                        {
                            scripting::item(scripting::item::types::rgb_8_bit_image, static_cast<std::uint32_t>(source) - 1),
                            scripting::item(scripting::item::types::signed_integer, static_cast<std::int32_t>(8))
                        }
                    };

                    result_id = parser->register_item(std::move(result_item));
                }
            }

            return result_id;
        };

    parser->register_specification(name(), std::move(fct2));
}

void input::on_compile(std::uint32_t item_id, std::shared_ptr<detail::compiler> compiler) const
{
    auto handler =
        detail::handler(
            [result_id = item_id, item = compiler->get_item(item_id)](std::shared_ptr<image_processor> image_processor, std::size_t context_id)
            {
                return ::detail::input(image_processor, context_id, result_id, std::move(item));
            });

    compiler->register_handler(item_id, name(), std::move(handler));
}

}}}} // namespace cvpg::imageproc::scripting::algorithms
