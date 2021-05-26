#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_TFPREDICT_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_TFPREDICT_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include <boost/asynchronous/servant_proxy.hpp>
#include <boost/asynchronous/trackable_servant.hpp>

#include <libcvpg/core/image.hpp>
#include <libcvpg/imageproc/scripting/diagnostics/typedefs.hpp>

#include <tensorflow/cc/saved_model/loader.h>
#include <tensorflow/core/public/session.h>

namespace cvpg::imageproc::algorithms {

class tfpredict_processor : public boost::asynchronous::trackable_servant<scripting::diagnostics::servant_job, scripting::diagnostics::servant_job>
{
public:
    using labels_type = std::unordered_map<std::size_t, std::string>;

    tfpredict_processor(boost::asynchronous::any_weak_scheduler<scripting::diagnostics::servant_job> scheduler, std::uint32_t threads);

    void load_model(std::string path, std::string input_layer, std::string output_layers, std::string extract_outputs, std::function<void(bool)> callback);

    void set_labels(labels_type labels);

    void process(cvpg::image_gray_8bit image, std::function<void(bool, std::string, cvpg::image_gray_8bit)> callback);
    void process(cvpg::image_rgb_8bit image, std::function<void(bool, std::string, cvpg::image_rgb_8bit)> callback);

private:
    std::uint32_t m_threads;

    std::unique_ptr<tensorflow::SavedModelBundleLite> m_model_bundle;

    std::string m_input_layer;
    std::string m_output_layers;

    std::string m_extract_outputs;

    labels_type m_labels;
};

struct tfpredict_processor_proxy : public boost::asynchronous::servant_proxy<tfpredict_processor_proxy, tfpredict_processor, scripting::diagnostics::servant_job>
{
    template<typename... Args>
    tfpredict_processor_proxy(Args... args)
        : boost::asynchronous::servant_proxy<tfpredict_processor_proxy, tfpredict_processor, scripting::diagnostics::servant_job>(std::forward<Args>(args)...)
    {}

    BOOST_ASYNC_POST_MEMBER_LOG(load_model, "load_model", 1)

    BOOST_ASYNC_POST_MEMBER_LOG(set_labels, "set_labels", 1)

    BOOST_ASYNC_POST_MEMBER_LOG(process, "process", 1)
};

} // namespace cvpg::imageproc::algorithms

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_TFPREDICT_HPP
