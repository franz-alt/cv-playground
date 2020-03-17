#include <libcvpg/imageproc/scripting/algorithms/multiply_add.hpp>

#include <chrono>
#include <functional>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/algorithms/tiling.hpp>
#include <libcvpg/imageproc/scripting/image_processor.hpp>
#include <libcvpg/imageproc/scripting/item.hpp>
#include <libcvpg/imageproc/scripting/detail/compiler.hpp>
#include <libcvpg/imageproc/scripting/detail/handler.hpp>
#include <libcvpg/imageproc/scripting/detail/parser.hpp>

namespace detail {

struct multiply_add_task :  public boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::image_processor> >
{
    multiply_add_task(std::shared_ptr<cvpg::imageproc::scripting::image_processor> image_processor, std::size_t context_id, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
        : boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::image_processor> >("algorithms::multiply_add_task")
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
            auto factor = std::any_cast<double>(m_item.arguments.at(1).value());
            auto offset = std::any_cast<std::int32_t>(m_item.arguments.at(2).value());

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

                auto tf = cvpg::imageproc::algorithms::tiling_functors::image<cvpg::image_gray_8bit>({{ std::move(image) }});
                tf.algorithm = cvpg::imageproc::algorithms::tiling_algorithms::multiply_add;
                tf.parameters.image_width = image.width();
                tf.parameters.image_height = image.height();
                tf.parameters.cutoff_x = cutoff_x;
                tf.parameters.cutoff_y = cutoff_y;
                tf.parameters.real_numbers.push_back(factor);
                tf.parameters.signed_integer_numbers.push_back(offset);

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
                    cvpg::imageproc::algorithms::tiling(std::move(tf))
                );
            }
            else if (input.type() == cvpg::imageproc::scripting::item::types::rgb_8_bit_image)
            {
                auto image = std::any_cast<cvpg::image_rgb_8bit>(input.value());

                auto start = std::chrono::system_clock::now();

                auto tf = cvpg::imageproc::algorithms::tiling_functors::image<cvpg::image_rgb_8bit>({{ std::move(image) }});
                tf.algorithm = cvpg::imageproc::algorithms::tiling_algorithms::multiply_add;
                tf.parameters.image_width = image.width();
                tf.parameters.image_height = image.height();
                tf.parameters.cutoff_x = cutoff_x;
                tf.parameters.cutoff_y = cutoff_y;
                tf.parameters.real_numbers.push_back(factor);
                tf.parameters.signed_integer_numbers.push_back(offset);

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
    std::shared_ptr<cvpg::imageproc::scripting::image_processor> m_image_processor;

    std::size_t m_context_id;

    std::uint32_t m_result_id;

    cvpg::imageproc::scripting::detail::parser::item m_item;
};

auto multiply_add(std::shared_ptr<cvpg::imageproc::scripting::image_processor> image_processor, std::size_t context_id, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
{
    return boost::asynchronous::top_level_callback_continuation<std::shared_ptr<cvpg::imageproc::scripting::image_processor> >(
               multiply_add_task(image_processor, context_id, result_id, std::move(item))
           );
}

} // namespace detail

namespace cvpg { namespace imageproc { namespace scripting { namespace algorithms {

std::string multiply_add::name() const
{
    return "multiply_add";
}

std::string multiply_add::category() const
{
    return "filters/arithmetic";
}

std::vector<parameter::item::item_type> multiply_add::result() const
{
    return
    {
        parameter::item::item_type::grayscale_8_bit_image,
        parameter::item::item_type::rgb_8_bit_image
    };
}

std::vector<std::vector<parameter> > multiply_add::parameters() const
{
    return std::vector<std::vector<parameter> >(
    {
        {
            parameter("image", "input image", "", parameter::item::item_type::grayscale_8_bit_image),
            parameter("factor", "multiplication factor", "", parameter::item::item_type::real),
            parameter("offset", "offset", "", parameter::item::item_type::signed_integer, in_range<std::int16_t>(-255, 255))
        },
        {
            parameter("image", "input image", "", parameter::item::item_type::rgb_8_bit_image),
            parameter("factor", "multiplication factor", "", parameter::item::item_type::real),
            parameter("offset", "offset", "", parameter::item::item_type::signed_integer, in_range<std::int16_t>(-255, 255))
        }
    });
}

std::vector<std::string> multiply_add::check_parameters(std::vector<std::any> parameters) const
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

void multiply_add::on_parse(std::shared_ptr<detail::parser> parser) const
{
    std::function<std::uint32_t(std::uint32_t, double, std::int32_t)> fct =
        [parser](std::uint32_t image_id, double factor, std::int32_t offset)
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
                                "multiply_add",
                                {
                                    scripting::item(scripting::item::types::grayscale_8_bit_image, image_id),
                                    scripting::item(scripting::item::types::real, factor),
                                    scripting::item(scripting::item::types::signed_integer, offset)
                                }
                            };

                            result_id = parser->register_item(std::move(result_item));

                            break;
                        }

                        case scripting::item::types::rgb_8_bit_image:
                        {
                            detail::parser::item result_item
                            {
                                "multiply_add",
                                {
                                    scripting::item(scripting::item::types::rgb_8_bit_image, image_id),
                                    scripting::item(scripting::item::types::real, factor),
                                    scripting::item(scripting::item::types::signed_integer, offset)
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

void multiply_add::on_compile(std::uint32_t item_id, std::shared_ptr<detail::compiler> compiler) const
{
    auto handler =
        detail::handler(
            [result_id = item_id, item = compiler->get_item(item_id)](std::shared_ptr<image_processor> image_processor, std::size_t context_id)
            {
                return ::detail::multiply_add(image_processor, context_id, result_id, std::move(item));
            });

    compiler->register_handler(item_id, name(), std::move(handler));
}

}}}} // namespace cvpg::imageproc::scripting::algorithms
