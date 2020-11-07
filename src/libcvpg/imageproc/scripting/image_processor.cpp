#include <libcvpg/imageproc/scripting/image_processor.hpp>

#include <exception>
#include <unordered_map>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/imageproc/algorithms/convert_to_gray.hpp>
#include <libcvpg/imageproc/algorithms/convert_to_rgb.hpp>
#include <libcvpg/imageproc/scripting/processing_context.hpp>
#include <libcvpg/imageproc/scripting/detail/handler.hpp>
#include <libcvpg/imageproc/scripting/detail/parallel_node.hpp>
#include <libcvpg/imageproc/scripting/detail/parser.hpp>
#include <libcvpg/imageproc/scripting/detail/sequence_node.hpp>
#include <libcvpg/imageproc/scripting/detail/single_node.hpp>

namespace {

struct executor_task : public boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::processing_context> >
{
    executor_task(cvpg::imageproc::scripting::detail::compiler::result compiled,
                  std::shared_ptr<cvpg::imageproc::scripting::processing_context> context)
        : boost::asynchronous::continuation_task<std::shared_ptr<cvpg::imageproc::scripting::processing_context>>("image_processor::evaluate::executor_task")
        , m_compiled(std::make_shared<cvpg::imageproc::scripting::detail::compiler::result>(std::move(compiled)))
        , m_context(context)
    {}

    void operator()()
    {
        auto callbacks = std::make_shared<std::vector<cvpg::imageproc::scripting::detail::handler::callback_type> >();

        get_callbacks(m_compiled->flow->cbegin(), m_compiled->flow->cend(), callbacks);

        visit(callbacks->begin(),
              callbacks->end(),
              this->this_task_result(),
              m_compiled,
              callbacks,
              m_context);
    }

private:
    template<class Iterator>
    void get_callbacks(Iterator it, Iterator end, std::shared_ptr<std::vector<cvpg::imageproc::scripting::detail::handler::callback_type> > callbacks)
    {
        if (it == end)
        {
            return;
        }

        auto * single = dynamic_cast<cvpg::imageproc::scripting::detail::single_node*>((*it).get());

        if (single != nullptr)
        {
            callbacks->push_back(m_compiled->handlers.at(single->get_item_id()));
        }
        else
        {
            auto * sequence_container = dynamic_cast<cvpg::imageproc::scripting::detail::sequence_node*>((*it).get());

            if (sequence_container != nullptr)
            {
                get_callbacks(sequence_container->cbegin(), sequence_container->cend(), callbacks);
            }
            else
            {
                auto * parallel_container = dynamic_cast<cvpg::imageproc::scripting::detail::parallel_node*>((*it).get());

                if (parallel_container != nullptr)
                {
                    get_callbacks(parallel_container->cbegin(), parallel_container->cend(), callbacks);
                }
            }
        }

        get_callbacks(++it, end, callbacks);
    }

    template<class Iterator, class Result>
    void visit(Iterator it,
               Iterator end,
               Result && result,
               std::shared_ptr<cvpg::imageproc::scripting::detail::compiler::result> compiled,
               std::shared_ptr<std::vector<cvpg::imageproc::scripting::detail::handler::callback_type> > callbacks,
               std::shared_ptr<cvpg::imageproc::scripting::processing_context> context)
    {
        boost::asynchronous::create_callback_continuation_job<cvpg::imageproc::scripting::diagnostics::servant_job>(
            [this
            ,it
            ,end
            ,result = std::forward<Result>(result)
            ,compiled
            ,callbacks
            ,context](auto cont_res) mutable
            {
                try
                {
                    std::get<0>(cont_res).get();

                    ++it;

                    if (it != end)
                    {
                        visit(it,
                              end,
                              std::forward<Result>(result),
                              compiled,
                              callbacks,
                              context);
                    }
                    else
                    {
                        result.set_value(std::move(context));
                    }
                }
                catch (std::exception const & e)
                {
                    result.set_exception(std::current_exception());
                }
                catch (...)
                {
                    result.set_exception(std::current_exception());
                }
            },
            (*it)(context)
        );
    }

    std::shared_ptr<cvpg::imageproc::scripting::detail::compiler::result> m_compiled;

    std::shared_ptr<cvpg::imageproc::scripting::processing_context> m_context;
};

auto executor(cvpg::imageproc::scripting::detail::compiler::result compiled,
              std::shared_ptr<cvpg::imageproc::scripting::processing_context> context)
{
    return boost::asynchronous::top_level_callback_continuation_job<std::shared_ptr<cvpg::imageproc::scripting::processing_context>, cvpg::imageproc::scripting::diagnostics::servant_job>(
               executor_task(std::move(compiled), context)
           );
}

}

namespace cvpg { namespace imageproc { namespace scripting {

image_processor::image_processor(boost::asynchronous::any_weak_scheduler<diagnostics::servant_job> scheduler,
                                 boost::asynchronous::any_shared_scheduler_proxy<diagnostics::servant_job> pool,
                                 algorithm_set algorithms)
    : boost::asynchronous::trackable_servant<diagnostics::servant_job, diagnostics::servant_job>(scheduler, pool)
    , m_algorithms(std::move(algorithms))
    , m_compiled()
    , m_compiled_hashes()
    , m_context()
    , m_context_counter(0)
    , m_params()
{}

void image_processor::compile(std::string expression, std::function<void(std::size_t)> successful_callback, std::function<void(std::size_t, std::string)> failed_callback)
{
    auto hash = std::hash<std::string>()(expression);

    try
    {
        auto it = m_compiled_hashes.find(hash);

        // process the expression with the compiler/parser if no hash is stored yet
        if (it == m_compiled_hashes.end())
        {
            auto compiler = std::make_shared<detail::compiler>(m_algorithms);
            auto parser = compiler->get_parser();

            parser->operator()(expression);

            auto compile_id = m_compiled.size();

            m_compiled.insert({ compile_id, std::move(compiler->operator()()) });
            m_compiled_hashes.insert({ hash, compile_id });

            successful_callback(compile_id);
        }
        else
        {
            successful_callback(it->second);
        }
    }
    catch (std::exception const & e)
    {
        failed_callback(0, e.what());
    }
}

void image_processor::evaluate(std::size_t compile_id, cvpg::image_gray_8bit image, std::function<void(item)> callback)
{
    auto it = m_compiled.find(compile_id);

    if (it != m_compiled.end())
    {
        auto compiled = it->second;

        const std::size_t context_id = m_context_counter++;

        auto context = std::make_shared<processing_context>(context_id);
        context->store(0, std::move(image));
        context->set_parameters(m_params);

        m_context.insert({ context_id, context });

        post_callback(
            [compiled = std::move(compiled)
            ,context]() mutable
            {
                return executor(std::move(compiled), context);
            },
            [this, context_id, callback](auto cont_res) mutable
            {
                auto item = std::move(cont_res.get())->load();

                this->m_context.erase(context_id);

                callback(std::move(item));
            },
            "scope::evaluate::image_gray_8bit",
            1,
            1
        );
    }
    else
    {
        // TODO error reporting!?

        callback(item());
    }
}

void image_processor::evaluate(std::size_t compile_id, cvpg::image_rgb_8bit image, std::function<void(item)> callback)
{
    auto it = m_compiled.find(compile_id);

    if (it != m_compiled.end())
    {
        auto compiled = it->second;

        const std::size_t context_id = m_context_counter++;

        auto context = std::make_shared<processing_context>(context_id);
        context->store(0, std::move(image));
        context->set_parameters(m_params);

        m_context.insert({ context_id, context });

        post_callback(
            [compiled = std::move(compiled)
            ,context]() mutable
            {
                return executor(std::move(compiled), context);
            },
            [this, context_id, callback](auto cont_res) mutable
            {
                auto item = std::move(cont_res.get())->load();

                this->m_context.erase(context_id);

                callback(std::move(item));
            },
            "image_processor::evaluate::image_rgb_8bit",
            1,
            1
        );
    }
    else
    {
        // TODO error reporting!?

        callback(item());
    }
}

void image_processor::evaluate_convert_if(std::size_t compile_id, cvpg::image_gray_8bit image, std::function<void(cvpg::image_gray_8bit)> callback, std::function<void(std::size_t, std::string)> failed_callback)
{
    evaluate(
        compile_id,
        std::move(image),
        [this, compile_id, callback = std::move(callback), failed_callback = std::move(failed_callback)](auto item) mutable
        {
            if (item.type() == cvpg::imageproc::scripting::item::types::grayscale_8_bit_image)
            {
                callback(std::move(std::any_cast<cvpg::image_gray_8bit>(std::move(item.value()))));
            }
            else if (item.type() == cvpg::imageproc::scripting::item::types::rgb_8_bit_image)
            {
                this->post_callback(
                    [image = std::move(std::any_cast<cvpg::image_rgb_8bit>(std::move(item.value())))]()
                    {
                        return imageproc::algorithms::convert_to_gray(std::move(image), imageproc::algorithms::rgb_conversion_mode::calc_average);
                    },
                    [compile_id, callback = std::move(callback), failed_callback = std::move(failed_callback)](auto cont_res) mutable
                    {
                        try
                        {
                            callback(std::move(cont_res.get()));
                        }
                        catch (std::exception const & e)
                        {
                            failed_callback(compile_id, e.what());
                        }
                    },
                    "image_processor::evaluate_convert_if::image_gray_8bit::callback",
                    1,
                    1
                );
            }
            else
            {
                // TODO error handling !!!!
            }
        }
    );
}

void image_processor::evaluate_convert_if(std::size_t compile_id, cvpg::image_rgb_8bit image, std::function<void(cvpg::image_rgb_8bit)> callback, std::function<void(std::size_t, std::string)> failed_callback)
{
    evaluate(
        compile_id,
        std::move(image),
        [this, compile_id, callback = std::move(callback), failed_callback = std::move(failed_callback)](auto item) mutable
        {
            if (item.type() == cvpg::imageproc::scripting::item::types::rgb_8_bit_image)
            {
                callback(std::move(std::any_cast<cvpg::image_rgb_8bit>(std::move(item.value()))));
            }
            else if (item.type() == cvpg::imageproc::scripting::item::types::grayscale_8_bit_image)
            {
                this->post_callback(
                    [image = std::move(std::any_cast<cvpg::image_gray_8bit>(std::move(item.value())))]()
                    {
                        return imageproc::algorithms::convert_to_rgb(std::move(image));
                    },
                    [compile_id, callback = std::move(callback), failed_callback = std::move(failed_callback)](auto cont_res) mutable
                    {
                        try
                        {
                            callback(std::move(cont_res.get()));
                        }
                        catch (std::exception const & e)
                        {
                            failed_callback(compile_id, e.what());
                        }
                    },
                    "image_processor::evaluate_convert_if::image_rgb_8bit::callback",
                    1,
                    1
                );
            }
            else
            {
                // TODO error handling !!!!
            }
        }
    );
}

void image_processor::evaluate(std::size_t compile_id, cvpg::image_gray_8bit image1, cvpg::image_gray_8bit image2, std::function<void(item)> callback)
{
    auto it = m_compiled.find(compile_id);

    if (it != m_compiled.end())
    {
        auto compiled = it->second;

        const std::size_t context_id = m_context_counter++;

        auto context = std::make_shared<processing_context>(context_id);
        context->store(0, std::move(image1));
        context->store(2, std::move(image2));
        context->set_parameters(m_params);

        m_context.insert({ context_id, context });

        post_callback(
            [compiled = std::move(compiled)
            ,context]() mutable
            {
                return executor(std::move(compiled), context);
            },
            [this, context_id, callback](auto cont_res) mutable
            {
                auto item = std::move(cont_res.get())->load();

                this->m_context.erase(context_id);

                callback(std::move(item));
            },
            "image_processor::evaluate::image_gray_8bit_2x",
            1,
            1
        );
    }
    else
    {
        // TODO error reporting!?

        callback(item());
    }
}

void image_processor::evaluate(std::size_t compile_id, cvpg::image_rgb_8bit image1, cvpg::image_rgb_8bit image2, std::function<void(item)> callback)
{
    auto it = m_compiled.find(compile_id);

    if (it != m_compiled.end())
    {
        auto compiled = it->second;

        const std::size_t context_id = m_context_counter++;

        auto context = std::make_shared<processing_context>(context_id);
        context->store(0, std::move(image1));
        context->store(2, std::move(image2));
        context->set_parameters(m_params);

        m_context.insert({ context_id, context });

        post_callback(
            [compiled = std::move(compiled)
            ,context]() mutable
            {
                return executor(std::move(compiled), context);
            },
            [this, context_id, callback](auto cont_res) mutable
            {
                auto item = std::move(cont_res.get())->load();

                this->m_context.erase(context_id);

                callback(std::move(item));
            },
            "image_processor::evaluate::image_rgb_8bit_2x",
            1,
            1
        );
    }
    else
    {
        // TODO error reporting!?

        callback(item());
    }
}

void image_processor::evaluate_convert_if(std::size_t compile_id, cvpg::image_gray_8bit image1, cvpg::image_gray_8bit image2, std::function<void(cvpg::image_gray_8bit)> callback, std::function<void(std::size_t, std::string)> failed_callback)
{
    evaluate(
        compile_id,
        std::move(image1),
        std::move(image2),
        [this, compile_id, callback = std::move(callback), failed_callback = std::move(failed_callback)](auto item) mutable
        {
            if (item.type() == cvpg::imageproc::scripting::item::types::grayscale_8_bit_image)
            {
                callback(std::move(std::any_cast<cvpg::image_gray_8bit>(std::move(item.value()))));
            }
            else if (item.type() == cvpg::imageproc::scripting::item::types::rgb_8_bit_image)
            {
                this->post_callback(
                    [image = std::move(std::any_cast<cvpg::image_rgb_8bit>(std::move(item.value())))]()
                    {
                        return imageproc::algorithms::convert_to_gray(std::move(image), imageproc::algorithms::rgb_conversion_mode::calc_average);
                    },
                    [compile_id, callback = std::move(callback), failed_callback = std::move(failed_callback)](auto cont_res) mutable
                    {
                        try
                        {
                            callback(std::move(cont_res.get()));
                        }
                        catch (std::exception const & e)
                        {
                            failed_callback(compile_id, e.what());
                        }
                    },
                    "image_processor::evaluate_convert_if::image_gray_8bit_2x::callback",
                    1,
                    1
                );
            }
            else
            {
                // TODO error handling !!!!
            }
        }
    );
}

void image_processor::evaluate_convert_if(std::size_t compile_id, cvpg::image_rgb_8bit image1, cvpg::image_rgb_8bit image2, std::function<void(cvpg::image_rgb_8bit)> callback, std::function<void(std::size_t, std::string)> failed_callback)
{
    evaluate(
        compile_id,
        std::move(image1),
        std::move(image2),
        [this, compile_id, callback = std::move(callback), failed_callback = std::move(failed_callback)](auto item) mutable
        {
            if (item.type() == cvpg::imageproc::scripting::item::types::rgb_8_bit_image)
            {
                callback(std::move(std::any_cast<cvpg::image_rgb_8bit>(std::move(item.value()))));
            }
            else if (item.type() == cvpg::imageproc::scripting::item::types::grayscale_8_bit_image)
            {
                this->post_callback(
                    [image = std::move(std::any_cast<cvpg::image_gray_8bit>(std::move(item.value())))]()
                    {
                        return imageproc::algorithms::convert_to_rgb(std::move(image));
                    },
                    [compile_id, callback = std::move(callback), failed_callback = std::move(failed_callback)](auto cont_res) mutable
                    {
                        try
                        {
                            callback(std::move(cont_res.get()));
                        }
                        catch (std::exception const & e)
                        {
                            failed_callback(compile_id, e.what());
                        }
                    },
                    "image_processor::evaluate_convert_if::image_rgb_8bit_2x::callback",
                    1,
                    1
                );
            }
            else
            {
                // TODO error handling !!!!
            }
        }
    );
}

void image_processor::add_param(std::string key, std::any value)
{
    m_params.insert({ std::move(key), std::move(value) });
}

void image_processor::parameters(std::function<void(parameters_type)> callback) const
{
    callback(m_params);
}

}}} // namespace cvpg::imageproc::scripting
