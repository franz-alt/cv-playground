#include <libcvpg/imageproc/scripting/algorithms/and.hpp>

#include <chrono>
#include <functional>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/core/exception.hpp>
#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/algorithms/tiling.hpp>
#include <libcvpg/imageproc/algorithms/tiling/and.hpp>
#include <libcvpg/imageproc/scripting/item.hpp>
#include <libcvpg/imageproc/scripting/processing_context.hpp>
#include <libcvpg/imageproc/scripting/detail/compiler.hpp>
#include <libcvpg/imageproc/scripting/detail/handler.hpp>
#include <libcvpg/imageproc/scripting/detail/parser.hpp>

namespace detail {

struct and_task : public boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >
{
    and_task(std::shared_ptr<cvpg::imageproc::scripting::processing_context> context, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
        : boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >("algorithms::and_task")
        , m_context(context)
        , m_result_id(result_id)
        , m_item(std::move(item))
    {}

    void operator()()
    {
        try
        {
            auto id1 = std::any_cast<std::uint32_t>(m_item.arguments.at(0).value());
            auto id2 = std::any_cast<std::uint32_t>(m_item.arguments.at(1).value());

            auto input1 = m_context->load(id1);
            auto input2 = m_context->load(id2);

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

            if (input1.type() == cvpg::imageproc::scripting::item::types::grayscale_8_bit_image &&
                input2.type() == cvpg::imageproc::scripting::item::types::grayscale_8_bit_image)
            {
                auto image1 = std::any_cast<cvpg::image_gray_8bit>(input1.value());
                auto image2 = std::any_cast<cvpg::image_gray_8bit>(input2.value());

                const auto width = image1.width();
                const auto height = image1.height();

                auto start = std::chrono::system_clock::now();

                auto tf = cvpg::imageproc::algorithms::tiling_functors::image<cvpg::image_gray_8bit>({{ std::move(image1), std::move(image2) }});
                tf.parameters.image_width = width;
                tf.parameters.image_height = height;
                tf.parameters.cutoff_x = cutoff_x;
                tf.parameters.cutoff_y = cutoff_y;

                tf.tile_algorithm_task = [](std::shared_ptr<cvpg::image_gray_8bit> src1, std::shared_ptr<cvpg::image_gray_8bit> src2, std::shared_ptr<cvpg::image_gray_8bit> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
                {
                    cvpg::imageproc::algorithms::and_gray_8bit(src1->data(0).get(), src2->data(0).get(), dst->data(0).get(), from_x, to_x, from_y, to_y, std::move(parameters));
                };

                boost::asynchronous::create_callback_continuation(
                    [result = this->this_task_result(), context = m_context, result_id = m_result_id, start](auto cont_res) mutable
                    {
                        auto stop = std::chrono::system_clock::now();

                        try
                        {
                            context->store(result_id, std::get<0>(cont_res).get(), std::chrono::duration_cast<std::chrono::microseconds>(stop - start));

                            result.set_value(context);
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
                tf.parameters.image_width = width;
                tf.parameters.image_height = height;
                tf.parameters.cutoff_x = cutoff_x;
                tf.parameters.cutoff_y = cutoff_y;

                tf.tile_algorithm_task = [](std::shared_ptr<cvpg::image_rgb_8bit> src1, std::shared_ptr<cvpg::image_rgb_8bit> src2, std::shared_ptr<cvpg::image_rgb_8bit> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
                {
                    cvpg::imageproc::algorithms::and_gray_8bit(src1->data(0).get(), src2->data(0).get(), dst->data(0).get(), from_x, to_x, from_y, to_y, parameters);
                    cvpg::imageproc::algorithms::and_gray_8bit(src1->data(1).get(), src2->data(1).get(), dst->data(1).get(), from_x, to_x, from_y, to_y, parameters);
                    cvpg::imageproc::algorithms::and_gray_8bit(src1->data(2).get(), src2->data(2).get(), dst->data(2).get(), from_x, to_x, from_y, to_y, std::move(parameters));
                };

                boost::asynchronous::create_callback_continuation(
                    [result = this->this_task_result(), context = m_context, result_id = m_result_id, start](auto cont_res) mutable
                    {
                        auto stop = std::chrono::system_clock::now();

                        try
                        {
                            context->store(result_id, std::get<0>(cont_res).get(), std::chrono::duration_cast<std::chrono::microseconds>(stop - start));

                            result.set_value(context);
                        }
                        catch (...)
                        {
                            result.set_exception(std::current_exception());
                        }
                    },
                    cvpg::imageproc::algorithms::tiling(std::move(tf))
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

auto and_(std::shared_ptr<cvpg::imageproc::scripting::processing_context> context, std::uint32_t result_id, cvpg::imageproc::scripting::detail::parser::item item)
{
    return boost::asynchronous::top_level_callback_continuation<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >(
               and_task(context, result_id, std::move(item))
           );
}

} // namespace detail

namespace cvpg { namespace imageproc { namespace scripting { namespace algorithms {

std::string and_::name() const
{
    return "and";
}

std::string and_::category() const
{
    return "filters/arithmetic";
}

std::vector<scripting::item::types> and_::result() const
{
    return
    {
        scripting::item::types::grayscale_8_bit_image,
        scripting::item::types::rgb_8_bit_image
    };
}

parameter_set and_::parameters() const
{
    return parameter_set
           ({
               parameter("image1", "first input image", "", { scripting::item::types::grayscale_8_bit_image, scripting::item::types::rgb_8_bit_image }),
               parameter("image2", "second input image", "", { scripting::item::types::grayscale_8_bit_image, scripting::item::types::rgb_8_bit_image })
           });
}

void and_::on_parse(std::shared_ptr<detail::parser> parser) const
{
    std::function<std::uint32_t(std::uint32_t, std::uint32_t)> fct =
        [parser, parameters = this->parameters()](std::uint32_t image1_id, std::uint32_t image2_id)
        {
            // find image
            if (!parser)
            {
                throw cvpg::invalid_parameter_exception("invalid parser");
            }

            auto image1 = parser->find_item(image1_id);
            auto image2 = parser->find_item(image2_id);

            if (image1.arguments.empty() || image2.arguments.empty())
            {
                throw cvpg::invalid_parameter_exception("invalid input ID");
            }

            auto input1_type = image1.arguments.front().type();
            auto input2_type = image2.arguments.front().type();

            // check parameters
            if (!((input1_type == scripting::item::types::grayscale_8_bit_image && input2_type == scripting::item::types::grayscale_8_bit_image) ||
                  (input1_type == scripting::item::types::rgb_8_bit_image && input2_type == scripting::item::types::rgb_8_bit_image)))
            {
                throw cvpg::invalid_parameter_exception("invalid input type");
            }

            std::uint32_t result_id = 0;

            detail::parser::item result_item
            {
                "and",
                {
                    scripting::item(image1.arguments.front().type(), image1_id),
                    scripting::item(image2.arguments.front().type(), image2_id)
                }
            };

            result_id = parser->register_item(std::move(result_item));

            parser->register_link(image1_id, result_id);
            parser->register_link(image2_id, result_id);

            return result_id;
        };

    parser->register_specification(name(), std::move(fct));
}

void and_::on_compile(std::uint32_t item_id, std::shared_ptr<detail::compiler> compiler) const
{
    auto handler =
        detail::handler(
            [result_id = item_id, item = compiler->get_item(item_id)](std::shared_ptr<processing_context> context)
            {
                return ::detail::and_(context, result_id, std::move(item));
            });

    compiler->register_handler(item_id, name(), std::move(handler));
}

}}}} // namespace cvpg::imageproc::scripting::algorithms
