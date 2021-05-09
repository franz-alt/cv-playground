#include <libcvpg/imageproc/scripting/algorithm_set.hpp>

#include <algorithm>

#include <libcvpg/imageproc/scripting/algorithms/and.hpp>
#include <libcvpg/imageproc/scripting/algorithms/base.hpp>
#include <libcvpg/imageproc/scripting/algorithms/binary_threshold.hpp>
#include <libcvpg/imageproc/scripting/algorithms/convert_to_gray.hpp>
#include <libcvpg/imageproc/scripting/algorithms/convert_to_rgb.hpp>
#include <libcvpg/imageproc/scripting/algorithms/diff.hpp>
#include <libcvpg/imageproc/scripting/algorithms/histogram_equalization.hpp>
#include <libcvpg/imageproc/scripting/algorithms/hog_image.hpp>
#include <libcvpg/imageproc/scripting/algorithms/input.hpp>
#include <libcvpg/imageproc/scripting/algorithms/k_means.hpp>
#include <libcvpg/imageproc/scripting/algorithms/mean.hpp>
#include <libcvpg/imageproc/scripting/algorithms/multiply_add.hpp>
#include <libcvpg/imageproc/scripting/algorithms/paint_meta.hpp>
#include <libcvpg/imageproc/scripting/algorithms/or.hpp>
#include <libcvpg/imageproc/scripting/algorithms/pooling.hpp>
#include <libcvpg/imageproc/scripting/algorithms/resize.hpp>
#include <libcvpg/imageproc/scripting/algorithms/scharr.hpp>
#include <libcvpg/imageproc/scripting/algorithms/sobel.hpp>
#include <libcvpg/imageproc/scripting/algorithms/threshold.hpp>

#ifdef USE_TENSORFLOW_CC
#include <libcvpg/imageproc/scripting/algorithms/tfpredict.hpp>
#endif

namespace cvpg { namespace imageproc { namespace scripting {

algorithm_set::algorithm_set()
    : m_specifications()
{
    register_algorithm(std::make_shared<algorithms::and_>());
    register_algorithm(std::make_shared<algorithms::binary_threshold>());
    register_algorithm(std::make_shared<algorithms::convert_to_gray>());
    register_algorithm(std::make_shared<algorithms::convert_to_rgb>());
    register_algorithm(std::make_shared<algorithms::diff>());
    register_algorithm(std::make_shared<algorithms::histogram_equalization>());
    register_algorithm(std::make_shared<algorithms::hog_image>());
    register_algorithm(std::make_shared<algorithms::input>());
    register_algorithm(std::make_shared<algorithms::k_means>());
    register_algorithm(std::make_shared<algorithms::mean>());
    register_algorithm(std::make_shared<algorithms::multiply_add>());
    register_algorithm(std::make_shared<algorithms::or_>());
    register_algorithm(std::make_shared<algorithms::paint_meta>());
    register_algorithm(std::make_shared<algorithms::pooling>());
    register_algorithm(std::make_shared<algorithms::resize>());
    register_algorithm(std::make_shared<algorithms::resize_to>());
    register_algorithm(std::make_shared<algorithms::scharr>());
    register_algorithm(std::make_shared<algorithms::sobel>());

#ifdef USE_TENSORFLOW_CC
    register_algorithm(std::make_shared<algorithms::tfpredict>());
#endif

    register_algorithm(std::make_shared<algorithms::threshold>());
}

std::shared_ptr<algorithms::base> algorithm_set::find(std::string name) const
{
    auto it = m_specifications.find(name);

    return (it != m_specifications.end()) ? it->second : std::shared_ptr<algorithms::base>();
}

std::vector<std::shared_ptr<algorithms::base> > algorithm_set::all() const
{
    std::vector<std::shared_ptr<algorithms::base> > result;

    std::transform(m_specifications.begin(),
                   m_specifications.end(),
                   std::back_inserter(result),
                   [](auto const & spec)
                   {
                       return spec.second;
                   });

    return result;
}

void algorithm_set::register_algorithm(std::shared_ptr<algorithms::base> specification)
{
    m_specifications.insert({ specification->name(), specification });
}

}}} // namespace cvpg::imageproc::scripting
