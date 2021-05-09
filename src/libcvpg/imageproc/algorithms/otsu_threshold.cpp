#include <libcvpg/imageproc/algorithms/otsu_threshold.hpp>

namespace cvpg::imageproc::algorithms {

std::uint8_t otsu_threshold(cvpg::histogram<std::size_t> const & h)
{
    double sum = 0.0;
    std::size_t total_pixels = 0;

    for (std::size_t i = 0; i < h.bins(); ++i)
    {
        sum += i * h.at(i);
        total_pixels += h.at(i);
    }

    double sum_background = 0.0;

    std::size_t w_background = 0;
    std::size_t w_foreground = 0;

    double variance_max = 0.0;
    std::uint8_t threshold = 0;

    for (std::size_t i = 0; i < h.bins(); ++i)
    {
        w_background += h.at(i);

        if (w_background == 0)
        {
            continue;
        }

        w_foreground = total_pixels - w_background;

        if (w_foreground == 0)
        {
            break;
        }

        sum_background += static_cast<double>(i * h.at(i));

        const double mean_background = sum_background / static_cast<double>(w_background);
        const double mean_foreground = (sum - sum_background) / static_cast<double>(w_foreground);

        // calculate between-class-variance
        const double variance = static_cast<double>(w_background) * static_cast<double>(w_foreground) * (mean_background - mean_foreground) * (mean_background - mean_foreground);

        // check if new maximum found
        if (variance > variance_max)
        {
            variance_max = variance;
            threshold = i;
        }
    }

    return threshold;
}

} // namespace cvpg::imageproc::algoritms
