#include <libcvpg/imageproc/algorithms/tfpredict.hpp>

#include <algorithm>
#include <numeric>
#include <vector>

#include <boost/algorithm/string.hpp>

#include <tensorflow/cc/saved_model/tag_constants.h>

#include <libcvpg/core/meta_data.hpp>
#include <libcvpg/core/multi_array.hpp>

namespace cvpg::imageproc::algorithms {

tfpredict_processor::tfpredict_processor(boost::asynchronous::any_weak_scheduler<scripting::diagnostics::servant_job> scheduler)
    : boost::asynchronous::trackable_servant<scripting::diagnostics::servant_job, scripting::diagnostics::servant_job>(scheduler)
{}

void tfpredict_processor::load_model(std::string path, std::string input_layer, std::string output_layers, std::string extract_outputs, std::function<void(bool)> callback)
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

    m_input_layer = std::move(input_layer);
    m_output_layers = std::move(output_layers);
    m_extract_outputs = std::move(extract_outputs);

    callback(status.ok());
}

void tfpredict_processor::process(cvpg::image_gray_8bit image, std::function<void(bool, std::string, cvpg::image_gray_8bit)> callback)
{
    // TODO

    callback(false, "not implemented", std::move(image));
}

void tfpredict_processor::process(cvpg::image_rgb_8bit image, std::function<void(bool, std::string, cvpg::image_rgb_8bit)> callback)
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
        callback(false, status.ToString(), std::move(image));
        return;
    }

    auto signatures = m_model_bundle->GetSignatures();

    auto it = std::find_if(signatures.begin(),
                           signatures.end(),
                           [](auto const & entry)
                           {
                               return entry.first == "serving_default";
                           });

    if (it == signatures.end())
    {
        callback(false, "no suitable output definitions found", std::move(image));
        return;
    }

    // extract the names of all output layers
    std::vector<std::string> output_names;

    auto const & definition = it->second;

    for (std::size_t i = 0; i < output_layers.size(); ++i)
    {
        // because definitions are stored at a map, we have to extract names depending on the right type 'StatefulPartitionedCall:...'
        auto definition_name = std::string("StatefulPartitionedCall:").append(std::to_string(i));

        auto it = std::find_if(definition.outputs().cbegin(),
                               definition.outputs().cend(),
                               [&definition_name](auto const & entry)
                               {
                                   return entry.second.name() == definition_name;
                               });

        if (it != definition.outputs().cend())
        {
            output_names.push_back(it->first);
        }
    }

    // check if output names and output tensors are of equal size
    if (output_names.size() != outputs.size())
    {
        callback(false, "amount of output names and output tensors are not of equal size", std::move(image));
        return;
    }

    std::vector<std::size_t> extracted_indices;

    if (m_extract_outputs == "*")
    {
        extracted_indices.resize(output_names.size());
        std::iota(extracted_indices.begin(), extracted_indices.end(), 0);
    }
    else
    {
        std::vector<std::string> extract_names;
        boost::split(extract_names, m_extract_outputs, boost::is_any_of(","));

        for (auto const & name : extract_names)
        {
            auto it = std::find_if(output_names.cbegin(),
                                   output_names.cend(),
                                   [&name](auto const & output_name)
                                   {
                                       return output_name == name;
                                   });

            if (it != output_names.cend())
            {
                extracted_indices.push_back(std::distance(output_names.cbegin(), it));
            }
        }
    }

    cvpg::meta_data metadata;
    metadata.push("extracted", output_names);

    for (auto const & index : extracted_indices)
    {
        auto const & tensor = outputs.at(index);

        if (tensor.dtype() == tensorflow::DT_FLOAT)
        {
            std::vector<int> dimensions;
            dimensions.reserve(tensor.dims());

            for (auto const & s : tensor.shape())
            {
                dimensions.push_back(s.size);
            }

            // we store dimensions in reverse order
            std::reverse(dimensions.begin(), dimensions.end());

            const std::size_t entries = std::accumulate(dimensions.cbegin(), dimensions.cend(), 1, std::multiplies<int>());

            std::vector<float> data;
            data.reserve(entries);

            if (dimensions.size() == 1)
            {
                auto output_tensor = outputs[index].tensor<float, 1>();

                for (int i = 0; i < dimensions[0]; ++i)
                {
                    data.push_back(output_tensor(i));
                }
            }
            else if (dimensions.size() == 2)
            {
                auto output_tensor = outputs[index].tensor<float, 2>();

                for (int i = 0; i < dimensions[0]; ++i)
                {
                    for (int j = 0; j < dimensions[1]; ++j)
                    {
                        data.push_back(output_tensor(j, i));
                    }
                }
            }
            else if (dimensions.size() == 3)
            {
                auto output_tensor = outputs[index].tensor<float, 3>();

                for (int i = 0; i < dimensions[0]; ++i)
                {
                    for (int j = 0; j < dimensions[1]; ++j)
                    {
                        for (int k = 0; k < dimensions[2]; ++k)
                        {
                            data.push_back(output_tensor(k, j, i));
                        }
                    }
                }
            }
            else
            {
                // TODO error handling
            }

            cvpg::multi_array<float> array(dimensions);
            array = std::move(data);

            metadata.push(std::string(output_names.at(index)).append(".data"), std::move(array));
            metadata.push(std::string(output_names.at(index)).append(".dims"), std::move(dimensions));
        }
    }

    image.set_metadata(std::make_shared<cvpg::meta_data>(std::move(metadata)));

    callback(true, "", std::move(image));
}

} // namespace cvpg::imageproc::algorithms
