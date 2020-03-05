#ifndef LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHM_SET_HPP
#define LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHM_SET_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace cvpg { namespace imageproc { namespace scripting {

namespace algorithms {

class base;

} // namespace algorithms

class algorithm_set
{
public:
    algorithm_set();

    std::shared_ptr<algorithms::base> find(std::string name) const;

    std::vector<std::shared_ptr<algorithms::base> > all() const;

private:
    void register_algorithm(std::shared_ptr<algorithms::base> specification);

    std::map<std::string, std::shared_ptr<algorithms::base> > m_specifications;
};

}}} // namespace cvpg::imageproc::scripting

#endif // LIBCVPG_IMAGEPROC_SCRIPTING_ALGORITHM_SET_HPP
