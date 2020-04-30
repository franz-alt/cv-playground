#include <libcvpg/imageproc/scripting/algorithms/parameter_set.hpp>

namespace cvpg { namespace imageproc { namespace scripting { namespace algorithms {

parameter_set::parameter_set(algorithms::parameter parameter)
    : m_parameters()
{
    m_parameters.push_back(std::move(parameter));
}

parameter_set::parameter_set(std::vector<parameter> parameters)
    : m_parameters(std::move(parameters))
{}

parameter_set::iterator_type parameter_set::begin() const
{
    return m_parameters.begin();
}

parameter_set::iterator_type parameter_set::end() const
{
    return m_parameters.end();
}

std::size_t parameter_set::size() const
{
    return m_parameters.size();
}

bool parameter_set::empty() const
{
    return m_parameters.empty();
}

}}}} // namespace cvpg::imageproc::scripting::algorithms
