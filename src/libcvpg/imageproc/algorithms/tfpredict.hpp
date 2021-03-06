#ifndef LIBCVPG_IMAGEPROC_ALGORITHMS_TFPREDICT_HPP
#define LIBCVPG_IMAGEPROC_ALGORITHMS_TFPREDICT_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

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
    tfpredict_processor(boost::asynchronous::any_weak_scheduler<scripting::diagnostics::servant_job> scheduler);

    void load_model(std::string path, std::uint32_t outputs, std::string extract_outputs, std::function<void(bool)> callback);

    void process(cvpg::image_gray_8bit image, std::function<void(bool, std::string)> callback);
    void process(cvpg::image_rgb_8bit image, std::function<void(bool, std::string)> callback);

private:
    std::unique_ptr<tensorflow::SavedModelBundleLite> m_model_bundle;

    std::uint32_t m_outputs = 0;

    std::string m_extract_outputs;
};

struct tfpredict_processor_proxy : public boost::asynchronous::servant_proxy<tfpredict_processor_proxy, tfpredict_processor, scripting::diagnostics::servant_job>
{
    template<typename... Args>
    tfpredict_processor_proxy(Args... args)
        : boost::asynchronous::servant_proxy<tfpredict_processor_proxy, tfpredict_processor, scripting::diagnostics::servant_job>(std::forward<Args>(args)...)
    {}

    BOOST_ASYNC_POST_MEMBER_LOG(load_model, "load_model", 1)

    BOOST_ASYNC_POST_MEMBER_LOG(process, "process", 1)
};

} // namespace cvpg::imageproc::algorithms

#endif // LIBCVPG_IMAGEPROC_ALGORITHMS_TFPREDICT_HPP
