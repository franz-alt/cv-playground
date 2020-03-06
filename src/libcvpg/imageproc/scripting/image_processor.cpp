#include <libcvpg/imageproc/scripting/image_processor.hpp>

#include <exception>
#include <unordered_map>

#include <boost/asynchronous/continuation_task.hpp>

#include <libcvpg/imageproc/scripting/detail/handler.hpp>
#include <libcvpg/imageproc/scripting/detail/parallel_node.hpp>
#include <libcvpg/imageproc/scripting/detail/parser.hpp>
#include <libcvpg/imageproc/scripting/detail/sequence_node.hpp>
#include <libcvpg/imageproc/scripting/detail/single_node.hpp>

namespace {

struct executor_task : public boost::asynchronous::continuation_task<cvpg::imageproc::scripting::item>
{
    executor_task(cvpg::imageproc::scripting::detail::compiler::result compiled,
                  std::shared_ptr<cvpg::imageproc::scripting::image_processor> image_processor,
                  std::size_t context_id)
        : boost::asynchronous::continuation_task<cvpg::imageproc::scripting::item>("image_processor::evaluate::executor_task")
        , m_compiled(std::make_shared<cvpg::imageproc::scripting::detail::compiler::result>(std::move(compiled)))
        , m_image_processor(image_processor)
        , m_context_id(context_id)
    {}

    void operator()()
    {
        auto callbacks = std::make_shared<std::vector<cvpg::imageproc::scripting::detail::handler::callback_type> >();

        get_callbacks(m_compiled->flow->cbegin(), m_compiled->flow->cend(), callbacks);

        visit(callbacks->begin(),
              callbacks->end(),
              this->this_task_result(),
              m_compiled,
              m_image_processor,
              m_context_id,
              callbacks);
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
               std::shared_ptr<cvpg::imageproc::scripting::image_processor> image_processor,
               std::size_t context_id,
               std::shared_ptr<std::vector<cvpg::imageproc::scripting::detail::handler::callback_type> > callbacks)
    {
        boost::asynchronous::create_callback_continuation_job<cvpg::imageproc::scripting::diagnostics::servant_job>(
            [this
            ,result = std::forward<Result>(result)
            ,it
            ,end
            ,compiled
            ,image_processor
            ,context_id
            ,callbacks](auto cont_res) mutable
            {
                try
                {
                    std::get<0>(cont_res).get();

                    cvpg::imageproc::scripting::item item;

                    if (std::distance(it, end) == 1)
                    {
                        item = image_processor->load(context_id);
                    }

                    ++it;

                    if (it != end)
                    {
                        visit(it,
                              end,
                              std::forward<Result>(result),
                              compiled,
                              image_processor,
                              context_id,
                              callbacks);
                    }
                    else
                    {
                        result.set_value(std::move(item));
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
            (*it)(image_processor, context_id)
        );
    }

    std::shared_ptr<cvpg::imageproc::scripting::detail::compiler::result> m_compiled;

    std::shared_ptr<cvpg::imageproc::scripting::image_processor> m_image_processor;

    std::size_t m_context_id;
};

auto executor(cvpg::imageproc::scripting::detail::compiler::result compiled,
              std::shared_ptr<cvpg::imageproc::scripting::image_processor> image_processor,
              std::size_t context_id)
{
    return boost::asynchronous::top_level_callback_continuation_job<cvpg::imageproc::scripting::item, cvpg::imageproc::scripting::diagnostics::servant_job>(
               executor_task(std::move(compiled), image_processor, context_id)
           );
}

}

namespace cvpg { namespace imageproc { namespace scripting {

struct image_processor::evaluation_context
{
    std::unordered_map<std::uint32_t, item> items;
    std::unordered_map<std::uint32_t, std::chrono::microseconds> durations;

    std::uint32_t last_stored = 0;
};

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

void image_processor::compile(std::string expression, std::function<void(bool, std::size_t)> callback)
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

            callback(true, compile_id);
        }
        else
        {
            callback(true, it->second);
        }
    }
    catch (std::exception const & e)
    {
        // TODO error reporting!?

        callback(false, 0);
    }
}

void image_processor::evaluate(std::size_t compile_id, cvpg::image_gray_8bit image, std::function<void(item)> callback)
{
    auto it = m_compiled.find(compile_id);

    if (it != m_compiled.end())
    {
        auto compiled = it->second;

        std::size_t context_id = m_context_counter++;

        m_context.insert({ context_id, std::make_shared<evaluation_context>() });

        store(context_id, 0, std::move(image));

        post_callback(
            [compiled = std::move(compiled)
            ,scope = shared_from_this()
            ,context_id]() mutable
            {
                return executor(std::move(compiled), scope, context_id);
            },
            [this, context_id, callback](auto cont_res) mutable
            {
                auto frame = cont_res.get();

                this->m_context.erase(context_id);

                callback(std::move(frame));
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

        std::size_t context_id = m_context_counter++;

        m_context.insert({ context_id, std::make_shared<evaluation_context>() });

        store(context_id, 0, std::move(image));

        post_callback(
            [compiled = std::move(compiled)
            ,scope = shared_from_this()
            ,context_id]() mutable
            {
                return executor(std::move(compiled), scope, context_id);
            },
            [this, context_id, callback](auto cont_res) mutable
            {
                auto item = cont_res.get();

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

void image_processor::evaluate(std::size_t compile_id, cvpg::image_gray_8bit image1, cvpg::image_gray_8bit image2, std::function<void(item)> callback)
{
    auto it = m_compiled.find(compile_id);

    if (it != m_compiled.end())
    {
        auto compiled = it->second;

        std::size_t context_id = m_context_counter++;

        m_context.insert({ context_id, std::make_shared<evaluation_context>() });

        store(context_id, 0, std::move(image1));
        store(context_id, 2, std::move(image2));

        post_callback(
            [compiled = std::move(compiled)
            ,scope = shared_from_this()
            ,context_id]() mutable
            {
                return executor(std::move(compiled), scope, context_id);
            },
            [this, context_id, callback](auto cont_res) mutable
            {
                auto item = cont_res.get();

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

void image_processor::store(std::size_t context_id, std::uint32_t image_id, cvpg::image_gray_8bit image, std::chrono::microseconds duration)
{
    m_context[context_id]->items[image_id] = item(item::types::grayscale_8_bit_image, std::move(image));
    m_context[context_id]->durations[image_id] = std::move(duration);
    m_context[context_id]->last_stored = image_id;
}

void image_processor::store(std::size_t context_id, std::uint32_t image_id, cvpg::image_rgb_8bit image, std::chrono::microseconds duration)
{
    m_context[context_id]->items[image_id] = item(item::types::rgb_8_bit_image, std::move(image));
    m_context[context_id]->durations[image_id] = std::move(duration);
    m_context[context_id]->last_stored = image_id;
}

item image_processor::load(std::size_t context_id, std::uint32_t image_id) const
{
    auto it = m_context.find(context_id);

    if (it != m_context.end())
    {
        auto & context = it->second;

        auto it = context->items.find(image_id);

        if (it != context->items.end())
        {
            return it->second;
        }
    }

    return item();
}

item image_processor::load(std::size_t context_id) const
{
    auto it = m_context.find(context_id);

    if (it != m_context.end())
    {
        auto & context = it->second;

        return load(context_id, context->last_stored);
    }

    return item();
}

void image_processor::add_param(std::string key, std::any value)
{
    m_params.insert({ std::move(key), std::move(value) });
}

image_processor::parameters_type image_processor::parameters() const
{
    return m_params;
}

}}} // namespace cvpg::imageproc::scripting
