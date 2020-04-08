#include <libcvpg/imageproc/scripting/algorithms/diff.hpp>

#include <chrono>
#include <functional>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/algorithms/tiling.hpp>
#include <libcvpg/imageproc/algorithms/tiling/diff.hpp>
#include <libcvpg/imageproc/scripting/image_processor.hpp>
#include <libcvpg/imageproc/scripting/item.hpp>
#include <libcvpg/imageproc/scripting/detail/compiler.hpp>
#include <libcvpg/imageproc/scripting/detail/handler.hpp>
#include <libcvpg/imageproc/scripting/detail/parser.hpp>

namespace detail {

struct diff_task : public boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::image_processor> >
{
    diff_task(std::shared_ptr<cvpg::imageproc::scripting::image_processor> image_processor, std::size_t context_id, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
        : boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::image_processor> >("algorithms::diff_task")
        , m_image_processor(image_processor)
        , m_context_id(context_id)
        , m_result_id(result_id)
        , m_item(std::move(item))
    {}

    void operator()()
    {        
        try
        {
            auto id1 = std::any_cast<std::uint32_t>(m_item.arguments.at(0).value());
            auto id2 = std::any_cast<std::uint32_t>(m_item.arguments.at(1).value());
            auto offset = std::any_cast<std::int32_t>(m_item.arguments.at(2).value());

            auto input1 = m_image_processor->load(m_context_id, id1);
            auto input2 = m_image_processor->load(m_context_id, id2);

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

            if (input1.type() == cvpg::imageproc::scripting::item::types::grayscale_8_bit_image &&
                input2.type() == cvpg::imageproc::scripting::item::types::grayscale_8_bit_image)
            {
                auto image1 = std::any_cast<cvpg::image_gray_8bit>(input1.value());
                auto image2 = std::any_cast<cvpg::image_gray_8bit>(input2.value());

                const auto width = image1.width();
                const auto height = image1.height();

                auto start = std::chrono::system_clock::now();

                auto tf = cvpg::imageproc::algorithms::tiling_functors::image<cvpg::image_gray_8bit>({{ std::move(image1), std::move(image2) }});
                tf.algorithm = cvpg::imageproc::algorithms::tiling_algorithms::diff;
                tf.parameters.image_width = width;
                tf.parameters.image_height = height;
                tf.parameters.cutoff_x = cutoff_x;
                tf.parameters.cutoff_y = cutoff_y;
                tf.parameters.signed_integer_numbers.push_back(offset);

                tf.tile_algorithm_task = [](std::shared_ptr<cvpg::image_gray_8bit> src1, std::shared_ptr<cvpg::image_gray_8bit> src2, std::shared_ptr<cvpg::image_gray_8bit> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_algorithms /*algorithm*/, cvpg::imageproc::algorithms::tiling_parameters parameters)
                {
                    cvpg::imageproc::algorithms::diff_gray_8bit(src1->data(0).get(), src2->data(0).get(), dst->data(0).get(), from_x, to_x, from_y, to_y, std::move(parameters));
                };

                boost::asynchronous::create_callback_continuation(
                    [result = this->this_task_result(), image_processor = m_image_processor, context_id = m_context_id, result_id = m_result_id, start](auto cont_res) mutable
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
            else if (input1.type() == cvpg::imageproc::scripting::item::types::rgb_8_bit_image &&
                     input2.type() == cvpg::imageproc::scripting::item::types::rgb_8_bit_image)
            {
                auto image1 = std::any_cast<cvpg::image_rgb_8bit>(input1.value());
                auto image2 = std::any_cast<cvpg::image_rgb_8bit>(input2.value());

                const auto width = image1.width();
                const auto height = image1.height();

                auto start = std::chrono::system_clock::now();

                auto tf = cvpg::imageproc::algorithms::tiling_functors::image<cvpg::image_rgb_8bit>({{ std::move(image1), std::move(image2) }});
                tf.algorithm = cvpg::imageproc::algorithms::tiling_algorithms::diff;
                tf.parameters.image_width = width;
                tf.parameters.image_height = height;
                tf.parameters.cutoff_x = cutoff_x;
                tf.parameters.cutoff_y = cutoff_y;
                tf.parameters.signed_integer_numbers.push_back(offset);

                tf.tile_algorithm_task = [](std::shared_ptr<cvpg::image_rgb_8bit> src1, std::shared_ptr<cvpg::image_rgb_8bit> src2, std::shared_ptr<cvpg::image_rgb_8bit> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_algorithms /*algorithm*/, cvpg::imageproc::algorithms::tiling_parameters parameters)
                {
                    cvpg::imageproc::algorithms::diff_gray_8bit(src1->data(0).get(), src2->data(0).get(), dst->data(0).get(), from_x, to_x, from_y, to_y, parameters);
                    cvpg::imageproc::algorithms::diff_gray_8bit(src1->data(1).get(), src2->data(1).get(), dst->data(1).get(), from_x, to_x, from_y, to_y, parameters);
                    cvpg::imageproc::algorithms::diff_gray_8bit(src1->data(2).get(), src2->data(2).get(), dst->data(2).get(), from_x, to_x, from_y, to_y, std::move(parameters));
                };

                boost::asynchronous::create_callback_continuation(
                    [result = this->this_task_result(), image_processor = m_image_processor, context_id = m_context_id, result_id = m_result_id, start](auto cont_res) mutable
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

auto diff(std::shared_ptr<cvpg::imageproc::scripting::image_processor> image_processor, std::size_t context_id, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
{
    return boost::asynchronous::top_level_callback_continuation<std::shared_ptr<cvpg::imageproc::scripting::image_processor> >(
               diff_task(image_processor, context_id, result_id, std::move(item))
           );
}

} // namespace detail

namespace cvpg { namespace imageproc { namespace scripting { namespace algorithms {

std::string diff::name() const
{
    return "diff";
}

std::string diff::category() const
{
    return "filters/arithmetic";
}

std::vector<parameter::item::item_type> diff::result() const
{
    return
    {
        parameter::item::item_type::grayscale_8_bit_image,
        parameter::item::item_type::rgb_8_bit_image
    };
}

std::vector<std::vector<parameter> > diff::parameters() const
{
    return std::vector<std::vector<parameter> >(
    {
        {
            parameter("image1", "input image", "", parameter::item::item_type::grayscale_8_bit_image),
            parameter("image2", "input image", "", parameter::item::item_type::grayscale_8_bit_image)
        },
        {
            parameter("image1", "input image", "", parameter::item::item_type::rgb_8_bit_image),
            parameter("image2", "input image", "", parameter::item::item_type::rgb_8_bit_image)
        }
    });
}

std::vector<std::string> diff::check_parameters(std::vector<std::any> parameters) const
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

void diff::on_parse(std::shared_ptr<detail::parser> parser) const
{
    std::function<std::uint32_t(std::uint32_t, std::uint32_t, std::int32_t)> fct =
        [parser](std::uint32_t image1_id, std::uint32_t image2_id, std::int32_t offset)
        {
            std::uint32_t result_id = 0;

            // find images
            if (!!parser)
            {
                auto image1 = parser->find_item(image1_id);
                auto image2 = parser->find_item(image2_id);

                if (image1.arguments.size() != 0 && image2.arguments.size() != 0 && image1.arguments.front().type() == image2.arguments.front().type())
                {
                    detail::parser::item result_item
                    {
                        "diff",
                        {
                            scripting::item(image1.arguments.front().type(), image1_id),
                            scripting::item(image2.arguments.front().type(), image2_id),
                            scripting::item(scripting::item::types::signed_integer, offset)
                        }
                    };

                    result_id = parser->register_item(std::move(result_item));

                    parser->register_link(image1_id, result_id);
                    parser->register_link(image2_id, result_id);
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

void diff::on_compile(std::uint32_t item_id, std::shared_ptr<detail::compiler> compiler) const
{
    auto handler =
        detail::handler(
            [result_id = item_id, item = compiler->get_item(item_id)](std::shared_ptr<image_processor> image_processor, std::size_t context_id)
            {
                return ::detail::diff(image_processor, context_id, result_id, std::move(item));
            });

    compiler->register_handler(item_id, name(), std::move(handler));
}

}}}} // namespace cvpg::imageproc::scripting::algorithms
