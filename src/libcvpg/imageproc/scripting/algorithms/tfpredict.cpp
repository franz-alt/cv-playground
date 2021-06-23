#include <libcvpg/imageproc/scripting/algorithms/tfpredict.hpp>

#include <exception>
#include <memory>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/exception.hpp>
#include <libcvpg/core/image.hpp>

#include <libcvpg/imageproc/algorithms/tfpredict.hpp>
#include <libcvpg/imageproc/scripting/processing_context.hpp>
#include <libcvpg/imageproc/scripting/detail/compiler.hpp>
#include <libcvpg/imageproc/scripting/detail/handler.hpp>
#include <libcvpg/imageproc/scripting/detail/parser.hpp>

namespace detail {

struct tfpredict_task :  public boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >
{
    tfpredict_task(std::shared_ptr<cvpg::imageproc::scripting::processing_context> context, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
        : boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >("algorithms::tfpredict")
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

            auto it = parameters.find("tfmodel_processor");

            if (it != parameters.end())
            {
                auto tfpredict_processor = std::any_cast<std::shared_ptr<cvpg::imageproc::algorithms::tfpredict_processor_proxy> >(it->second);

                if (input.type() == cvpg::imageproc::scripting::item::types::grayscale_8_bit_image)
                {
                    auto image = std::any_cast<cvpg::image_gray_8bit>(std::move(input.value()));

                    auto start = std::chrono::system_clock::now();

                    tfpredict_processor->process(
                        std::move(image),
                        [result = this->this_task_result(), context = m_context, result_id = m_result_id, start](bool status, std::string message, cvpg::image_gray_8bit image)
                        {
                            auto stop = std::chrono::system_clock::now();

                            if (!status)
                            {
                                throw std::runtime_error(std::move(message));
                            }

                            context->store(result_id, std::move(image), std::chrono::duration_cast<std::chrono::microseconds>(stop - start));

                            result.set_value(context);
                        }
                    );
                }
                else if (input.type() == cvpg::imageproc::scripting::item::types::rgb_8_bit_image)
                {
                    auto image = std::any_cast<cvpg::image_rgb_8bit>(std::move(input.value()));

                    auto start = std::chrono::system_clock::now();

                    tfpredict_processor->process(
                        std::move(image),
                        [result = this->this_task_result(), context = m_context, result_id = m_result_id, start](bool status, std::string message, cvpg::image_rgb_8bit image)
                        {
                            auto stop = std::chrono::system_clock::now();

                            if (!status)
                            {
                                throw std::runtime_error(std::move(message));
                            }

                            context->store(result_id, std::move(image), std::chrono::duration_cast<std::chrono::microseconds>(stop - start));

                            result.set_value(context);
                        }
                    );
                }
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

auto tfpredict(std::shared_ptr<cvpg::imageproc::scripting::processing_context> context, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
{
    return boost::asynchronous::top_level_callback_continuation<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >(
               tfpredict_task(context, result_id, std::move(item))
           );
}

} // namespace detail

namespace cvpg::imageproc::scripting::algorithms {

std::string tfpredict::name() const
{
    return "tfpredict";
}

std::string tfpredict::category() const
{
    return "filters/segmentation";
}

std::vector<scripting::item::types> tfpredict::result() const
{
    return
    {
        scripting::item::types::grayscale_8_bit_image,
        scripting::item::types::rgb_8_bit_image,
    };
}

parameter_set tfpredict::parameters() const
{
    using namespace std::string_literals;

    return parameter_set
           ({
               parameter("image", "input image", "", { scripting::item::types::grayscale_8_bit_image, scripting::item::types::rgb_8_bit_image })
           });
}

void tfpredict::on_parse(std::shared_ptr<detail::parser> parser) const
{
    std::function<std::uint32_t(std::uint32_t)> fct =
        [parser, parameters = this->parameters()](std::uint32_t image_id)
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

            std::uint32_t result_id = 0;

            switch (input_type)
            {
                case scripting::item::types::grayscale_8_bit_image:
                {
                    detail::parser::item result_item
                    {
                        "tfpredict",
                        {
                            scripting::item(scripting::item::types::grayscale_8_bit_image, image_id)
                        }
                    };

                    result_id = parser->register_item(std::move(result_item));

                    break;
                }

                case scripting::item::types::rgb_8_bit_image:
                {
                    detail::parser::item result_item
                    {
                        "tfpredict",
                        {
                            scripting::item(scripting::item::types::rgb_8_bit_image, image_id)
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

void tfpredict::on_compile(std::uint32_t item_id, std::shared_ptr<detail::compiler> compiler) const
{
    auto handler =
        detail::handler(
            [result_id = item_id, item = compiler->get_item(item_id)](std::shared_ptr<processing_context> context)
            {
                return ::detail::tfpredict(context, result_id, std::move(item));
            });

    compiler->register_handler(item_id, name(), std::move(handler));
}

} // namespace cvpg::imageproc::scripting::algorithms
