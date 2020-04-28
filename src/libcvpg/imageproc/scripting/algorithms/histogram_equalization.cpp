#include <libcvpg/imageproc/scripting/algorithms/histogram_equalization.hpp>

#include <chrono>
#include <functional>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/exception.hpp>
#include <libcvpg/imageproc/algorithms/histogram_equalization.hpp>
#include <libcvpg/imageproc/scripting/image_processor.hpp>
#include <libcvpg/imageproc/scripting/item.hpp>
#include <libcvpg/imageproc/scripting/detail/compiler.hpp>
#include <libcvpg/imageproc/scripting/detail/handler.hpp>
#include <libcvpg/imageproc/scripting/detail/parser.hpp>

namespace detail {

struct histogram_equalization_task :  public boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::image_processor> >
{
    histogram_equalization_task(std::shared_ptr<cvpg::imageproc::scripting::image_processor> image_processor, std::size_t context_id, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
        : boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::image_processor> >("algorithms::histogram_equalization_task")
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

            if (input.type() == cvpg::imageproc::scripting::item::types::grayscale_8_bit_image)
            {
                auto image = std::any_cast<cvpg::image_gray_8bit>(input.value());

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
    std::shared_ptr<cvpg::imageproc::scripting::image_processor> m_image_processor;

    std::size_t m_context_id;

    std::uint32_t m_result_id;

    cvpg::imageproc::scripting::detail::parser::item m_item;
};

auto histogram_equalization(std::shared_ptr<cvpg::imageproc::scripting::image_processor> image_processor, std::size_t context_id, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
{
    return boost::asynchronous::top_level_callback_continuation<std::shared_ptr<cvpg::imageproc::scripting::image_processor> >(
               histogram_equalization_task(image_processor, context_id, result_id, std::move(item))
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
                                "histogram_equalization",
                                {
                                    scripting::item(scripting::item::types::grayscale_8_bit_image, image_id)
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

void histogram_equalization::on_compile(std::uint32_t item_id, std::shared_ptr<detail::compiler> compiler) const
{
    auto handler =
        detail::handler(
            [result_id = item_id, item = compiler->get_item(item_id)](std::shared_ptr<image_processor> image_processor, std::size_t context_id)
            {
                return ::detail::histogram_equalization(image_processor, context_id, result_id, std::move(item));
            });

    compiler->register_handler(item_id, name(), std::move(handler));
}

}}}} // namespace cvpg::imageproc::scripting::algorithms
