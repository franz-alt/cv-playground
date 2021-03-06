#include <libcvpg/imageproc/algorithms/tfpredict.hpp>

#include <algorithm>

#include <tensorflow/cc/saved_model/tag_constants.h>

namespace cvpg::imageproc::algorithms {

tfpredict_processor::tfpredict_processor(boost::asynchronous::any_weak_scheduler<scripting::diagnostics::servant_job> scheduler)
    : boost::asynchronous::trackable_servant<scripting::diagnostics::servant_job, scripting::diagnostics::servant_job>(scheduler)
{}

void tfpredict_processor::load_model(std::string path, uint32_t outputs, std::string extract_outputs, std::function<void(bool)> callback)
{
    m_model_bundle = std::make_unique<tensorflow::SavedModelBundleLite>();
    tensorflow::SessionOptions session_options = tensorflow::SessionOptions();
    tensorflow::ConfigProto & config = session_options.config;
    tensorflow::RunOptions run_options = tensorflow::RunOptions();
    tensorflow::Status status = tensorflow::LoadSavedModel(session_options, run_options, path, { tensorflow::kSavedModelTagServe }, m_model_bundle.get());

    if (!status.ok())
    {
        std::cout << status.ToString() << std::endl;
    }

    m_outputs = outputs;
    m_extract_outputs = std::move(extract_outputs);

    callback(status.ok());
}

void tfpredict_processor::process(cvpg::image_gray_8bit image, std::function<void(bool, std::string)> callback)
{
    // TODO

    callback(false, "not implemented");
}

void tfpredict_processor::process(cvpg::image_rgb_8bit image, std::function<void(bool, std::string)> callback)
{
    auto session = m_model_bundle->GetSession();

    tensorflow::Tensor input_tensor(tensorflow::DT_UINT8, tensorflow::TensorShape({ 1, image.width(), image.height(), 3 }));

    // copy image data to tensor
    auto input_tensor_mapped = input_tensor.tensor<std::uint8_t, 4>();

    auto data_r = image.data(0).get();
    auto data_g = image.data(1).get();
    auto data_b = image.data(2).get();

    for (auto y = 0; y < image.height(); ++y)
    {
        for (auto x = 0; x < image.width(); ++x)
        {
            // TODO is there a way to copy via memcpy !?!?
            input_tensor_mapped(0, x, y, 0) = data_r[x + y * image.width()];
            input_tensor_mapped(0, x, y, 1) = data_g[x + y * image.width()];
            input_tensor_mapped(0, x, y, 2) = data_b[x + y * image.width()];
        }
    }

    std::vector<std::string> output_layers(m_outputs);
    std::generate(output_layers.begin(), output_layers.end(), [n = 0]() mutable { return std::string("StatefulPartitionedCall:").append(std::to_string(n++)); });

    std::vector<tensorflow::Tensor> outputs;
    outputs.resize(8);

    auto status = session->Run({{ "serving_default_input_tensor:0", input_tensor }}, output_layers, {}, &outputs);

    if (!status.ok())
    {
        callback(false, status.ToString());
        return;
    }

    // TODO implement me!

    callback(true, "");
}

} // namespace cvpg::imageproc::algorithms
