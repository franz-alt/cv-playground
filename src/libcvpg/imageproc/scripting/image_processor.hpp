#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_IMAGE_PROCESSOR_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_IMAGE_PROCESSOR_HPP

#include <any>
#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>

#include <boost/asynchronous/servant_proxy.hpp>
#include <boost/asynchronous/trackable_servant.hpp>

#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/scripting/algorithm_set.hpp>
#include <libcvpg/imageproc/scripting/item.hpp>
#include <libcvpg/imageproc/scripting/detail/compiler.hpp>
#include <libcvpg/imageproc/scripting/diagnostics/typedefs.hpp>

namespace cvpg { namespace imageproc { namespace scripting {

class image_processor : public boost::asynchronous::trackable_servant<diagnostics::servant_job, diagnostics::servant_job>
                      , public std::enable_shared_from_this<image_processor>
{
public:
    using parameters_type = std::map<std::string, std::any>;

    image_processor(boost::asynchronous::any_weak_scheduler<diagnostics::servant_job> scheduler,
                    boost::asynchronous::any_shared_scheduler_proxy<diagnostics::servant_job> pool,
                    algorithm_set algorithms = algorithm_set());

    image_processor(image_processor const &) = delete;
    image_processor(image_processor &&) = default;

    image_processor & operator=(image_processor const &) = delete;
    image_processor & operator=(image_processor &&) = default;

    // compile an expression and return if compile was successfull and the compile ID
    void compile(std::string expression, std::function<void(std::size_t)> successful_callback, std::function<void(std::size_t, std::string)> failed_callback);

    // evaluate an input image
    void evaluate(std::size_t compile_id, cvpg::image_gray_8bit image, std::function<void(item)> callback);
    void evaluate(std::size_t compile_id, cvpg::image_rgb_8bit image, std::function<void(item)> callback);

    // evaluate an input image and convert the result if it is not the same as the input type
    void evaluate_convert_if(std::size_t compile_id, cvpg::image_gray_8bit image, std::function<void(cvpg::image_gray_8bit)> callback);
    void evaluate_convert_if(std::size_t compile_id, cvpg::image_rgb_8bit image, std::function<void(cvpg::image_rgb_8bit)> callback);

    // evaluate two input images
    void evaluate(std::size_t compile_id, cvpg::image_gray_8bit image1, cvpg::image_gray_8bit image2, std::function<void(item)> callback);
    void evaluate(std::size_t compile_id, cvpg::image_rgb_8bit image1, cvpg::image_rgb_8bit image2, std::function<void(item)> callback);

    // evaluate two input images and convert the result if it is not the same as the input type
    void evaluate_convert_if(std::size_t compile_id, cvpg::image_gray_8bit image1, cvpg::image_gray_8bit image2, std::function<void(cvpg::image_gray_8bit)> callback);
    void evaluate_convert_if(std::size_t compile_id, cvpg::image_rgb_8bit image1, cvpg::image_rgb_8bit image2, std::function<void(cvpg::image_rgb_8bit)> callback);

    // store an image
    void store(std::size_t context_id, std::uint32_t image_id, cvpg::image_gray_8bit image, std::chrono::microseconds duration = std::chrono::microseconds());
    void store(std::size_t context_id, std::uint32_t image_id, cvpg::image_rgb_8bit image, std::chrono::microseconds duration = std::chrono::microseconds());

    // load an item with a specific ID
    item load(std::size_t context_id, std::uint32_t image_id) const;

    // load last stored item
    item load(std::size_t context_id) const;

    // add a parameter for filters
    void add_param(std::string key, std::any value);

    // get all parameters
    parameters_type parameters() const;

private:
    algorithm_set m_algorithms;

    std::map<std::size_t, detail::compiler::result> m_compiled;
    std::map<std::size_t, std::size_t> m_compiled_hashes;

    struct evaluation_context;
    std::map<std::size_t, std::shared_ptr<evaluation_context> > m_context;

    std::size_t m_context_counter;

    parameters_type m_params;
};

struct image_processor_proxy : public boost::asynchronous::servant_proxy<image_processor_proxy, image_processor, diagnostics::servant_job>
{
   template<typename... Args>
   image_processor_proxy(Args... args)
       : boost::asynchronous::servant_proxy<image_processor_proxy, image_processor, diagnostics::servant_job>(std::forward<Args>(args)...)
   {}

   BOOST_ASYNC_POST_MEMBER_LOG(compile, "compile", 1)

   BOOST_ASYNC_POST_MEMBER_LOG(evaluate, "evaluate", 1)
   BOOST_ASYNC_POST_MEMBER_LOG(evaluate_convert_if, "evaluate_convert_if", 1)

   BOOST_ASYNC_FUTURE_MEMBER_LOG(load, "load", 1)

   BOOST_ASYNC_POST_MEMBER_LOG(add_param, "add_param", 1)
   BOOST_ASYNC_POST_MEMBER_LOG(parameters, "parameters", 1)
};

}}} // namespace cvpg::imageproc::scripting

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_IMAGE_PROCESSOR_HPP
