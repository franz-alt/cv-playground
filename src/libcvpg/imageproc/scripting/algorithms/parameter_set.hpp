// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHMS_PARAMETER_SET_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHMS_PARAMETER_SET_HPP

#include <algorithm>
#include <iterator>
#include <vector>

#include <libcvpg/imageproc/scripting/algorithms/parameter.hpp>

namespace cvpg::imageproc::scripting::algorithms {

class parameter_set
{
public:
    using iterator_type = std::vector<parameter>::const_iterator;

    parameter_set(algorithms::parameter parameter);
    
    parameter_set(std::vector<parameter> parameters);

    iterator_type begin() const;
    iterator_type end() const;

    std::size_t size() const;

    bool empty() const;

    template<typename T>
    bool is_valid(std::string parameter, T value) const
    {
        auto it = std::find_if(m_parameters.begin(),
                               m_parameters.end(),
                               [parameter](auto const & p)
                               {
                                   return p.name() == parameter;
                               });

        if (it != m_parameters.end())
        {
            return it->is_valid(value);
        }

        return false;
    }

private:
    std::vector<parameter> m_parameters;
};

} // namespace cvpg::imageproc::scripting::algorithms

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHMS_PARAMETER_SET_HPP
