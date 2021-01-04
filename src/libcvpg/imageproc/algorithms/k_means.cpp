#include <libcvpg/imageproc/algorithms/k_means.hpp>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>
#include <random>
#include <vector>

#include <boost/asynchronous/algorithm/then.hpp>

#include <libcvpg/core/exception.hpp>
#include <libcvpg/core/histogram.hpp>
#include <libcvpg/imageproc/algorithms/tiling.hpp>
#include <libcvpg/imageproc/algorithms/tiling/histogram.hpp>
#include <libcvpg/imageproc/algorithms/tiling/functors/histogram.hpp>

namespace {

void histogram_if_gray_8bit(std::uint8_t * src, std::vector<std::size_t> * dst, std::uint8_t * predicate_img, std::function<bool(std::uint8_t const & cluster)> predicate, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
{
    const std::size_t image_width = parameters.image_width;

    std::uint8_t * src_line = nullptr;
    std::uint8_t * predicate_line = nullptr;

    for (std::size_t y = from_y; y <= to_y; ++y)
    {
        src_line = src + image_width * y;
        predicate_line = predicate_img + image_width * y;

        for (std::size_t x = from_x; x <= to_x; ++x)
        {
            if (predicate(predicate_line[x]))
            {
                ++((*dst)[src_line[x]]);
            }
        }
    }
}

struct merge_histograms_task : public boost::asynchronous::continuation_task<std::shared_ptr<std::vector<cvpg::histogram<std::size_t> > >>
{
    merge_histograms_task(std::shared_ptr<std::vector<cvpg::histogram<std::size_t> > > a, std::shared_ptr<std::vector<cvpg::histogram<std::size_t> > > b)
        : boost::asynchronous::continuation_task<std::shared_ptr<std::vector<cvpg::histogram<std::size_t> > > >("k_means_task::merge_histograms_task")
        , m_a(a)
        , m_b(b)
    {}

    void operator()()
    {
        try
        {
            if (m_a->size() != m_b->size())
            {
                throw cvpg::exception("cannot merge histograms of different sizes");
            }

            std::vector<cvpg::histogram<std::size_t> > histograms;
            histograms.reserve(m_a->size());

            for (std::size_t i = 0; i < m_a->size(); ++i)
            {
                histograms.push_back(m_a->at(i) + m_b->at(i));
            }

            this_task_result().set_value(std::make_shared<std::vector<cvpg::histogram<std::size_t> > >(std::move(histograms)));
        }
        catch (...)
        {
            this_task_result().set_exception(std::current_exception());
        }
    }

private:
    std::shared_ptr<std::vector<cvpg::histogram<std::size_t> > > m_a;
    std::shared_ptr<std::vector<cvpg::histogram<std::size_t> > > m_b;
};

boost::asynchronous::detail::callback_continuation<std::shared_ptr<std::vector<cvpg::histogram<std::size_t> > > >
merge_histograms(std::shared_ptr<std::vector<cvpg::histogram<std::size_t> > > a, std::shared_ptr<std::vector<cvpg::histogram<std::size_t> > > b)
{
    return boost::asynchronous::top_level_callback_continuation<std::shared_ptr<std::vector<cvpg::histogram<std::size_t> > > >(
               merge_histograms_task(a, b)
           );
}

struct point
{
    point() = default;

    point(std::uint8_t x_, std::uint8_t y_, std::uint8_t z_)
        : x(x_)
        , y(y_)
        , z(z_)
    {}

    std::uint8_t x = 0;
    std::uint8_t y = 0;
    std::uint8_t z = 0;
};

void determine_cluster_gray_8bit(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters, std::vector<point> centers)
{
    const std::size_t image_width = parameters.image_width;
    const std::size_t clusters = centers.size();

    std::vector<std::size_t> distances(clusters, 0);

    std::uint8_t * src_line = nullptr;
    std::uint8_t * dst_line = nullptr;

    for (std::size_t y = from_y; y <= to_y; ++y)
    {
        const std::size_t offset_y = image_width * y;

        src_line = src + offset_y;
        dst_line = dst + offset_y;

        for (std::size_t x = from_x; x <= to_x; ++x)
        {
            const point p = { src_line[x], 0, 0 };

            for (std::size_t i = 0; i < clusters; ++i)
            {
                const point & c = centers[i];

                const double dxs = static_cast<double>(static_cast<std::int16_t>(p.x) - static_cast<std::int16_t>(c.x));

                // calculate the euclidian distance between current pixel and center of current cluster
                distances[i] = static_cast<std::size_t>(std::abs(dxs));
            }

            // determine index of cluster with the smallest distance to current pixel (in color space) for c=[1..k]
            const std::uint8_t c = static_cast<std::uint8_t>(std::distance(distances.begin(), std::min_element(distances.begin(), distances.end()))) + 1;

            // write cluster number to destination image
            dst_line[x] = c;
        }
    }
}

void determine_cluster_rgb_8bit(std::uint8_t * src_red, std::uint8_t * src_green, std::uint8_t * src_blue, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters, std::vector<point> centers)
{
    const std::size_t image_width = parameters.image_width;
    const std::size_t clusters = centers.size();

    std::vector<std::size_t> distances(clusters, 0);

    std::uint8_t * src_red_line = nullptr;
    std::uint8_t * src_green_line = nullptr;
    std::uint8_t * src_blue_line = nullptr;
    std::uint8_t * dst_line = nullptr;

    for (std::size_t y = from_y; y <= to_y; ++y)
    {
        const std::size_t offset_y = image_width * y;

        src_red_line = src_red + offset_y;
        src_green_line = src_green + offset_y;
        src_blue_line = src_blue + offset_y;
        dst_line = dst + offset_y;

        for (std::size_t x = from_x; x <= to_x; ++x)
        {
            const point p = { src_red_line[x], src_green_line[x], src_blue_line[x] };

            for (std::size_t i = 0; i < clusters; ++i)
            {
                const point & c = centers[i];

                const double dxs = static_cast<double>(static_cast<std::int16_t>(p.x) - static_cast<std::int16_t>(c.x));
                const double dys = static_cast<double>(static_cast<std::int16_t>(p.y) - static_cast<std::int16_t>(c.y));
                const double dzs = static_cast<double>(static_cast<std::int16_t>(p.z) - static_cast<std::int16_t>(c.z));

                // calculate the euclidian distance between current pixel and center of current cluster
                distances[i] = static_cast<std::size_t>(std::sqrt(dxs * dxs + dys * dys + dzs * dzs));
            }

            // determine index of cluster with the smallest distance to current pixel (in color space) for c=[1..k]
            const std::uint8_t c = static_cast<std::uint8_t>(std::distance(distances.begin(), std::min_element(distances.begin(), distances.end()))) + 1;

            // write cluster number to destination image
            dst_line[x] = c;
        }
    }
}

template<class image_type>
struct determine_cluster_task : public boost::asynchronous::continuation_task<cvpg::image_gray_8bit> {};

template<>
struct determine_cluster_task<cvpg::image_gray_8bit> : public boost::asynchronous::continuation_task<cvpg::image_gray_8bit>
{
    determine_cluster_task(std::shared_ptr<cvpg::image_gray_8bit> image, std::vector<point> centers)
        : boost::asynchronous::continuation_task<cvpg::image_gray_8bit>("k_means_task::determine_cluster_task")
        , m_image(std::move(image))
        , m_centers(std::move(centers))
    {}

    void operator()()
    {
        auto tf = cvpg::imageproc::algorithms::tiling_functors::image<cvpg::image_gray_8bit, cvpg::image_gray_8bit>({{ *m_image }});
        tf.parameters.image_width = m_image->width();
        tf.parameters.image_height = m_image->height();
        tf.parameters.cutoff_x = 512; // TODO use parameter
        tf.parameters.cutoff_y = 512; // TODO use parameter

        tf.tile_algorithm_task = [centers = m_centers](std::shared_ptr<cvpg::image_gray_8bit> src1, std::shared_ptr<cvpg::image_gray_8bit> /*src2*/, std::shared_ptr<cvpg::image_gray_8bit> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
        {
            determine_cluster_gray_8bit(src1->data(0).get(), dst->data(0).get(), from_x, to_x, from_y, to_y, std::move(parameters), std::move(centers));
        };

        boost::asynchronous::create_callback_continuation(
            [result = this->this_task_result()](auto cont_res)
            {
                try
                {
                    result.set_value(std::get<0>(cont_res).get());
                }
                catch (...)
                {
                    result.set_exception(std::current_exception());
                }
            },
            cvpg::imageproc::algorithms::tiling(std::move(tf))
        );
    }

private:
    std::shared_ptr<cvpg::image_gray_8bit> m_image;

    std::vector<point> m_centers;
};

template<>
struct determine_cluster_task<cvpg::image_rgb_8bit> : public boost::asynchronous::continuation_task<cvpg::image_gray_8bit>
{
    determine_cluster_task(std::shared_ptr<cvpg::image_rgb_8bit> image, std::vector<point> centers)
        : boost::asynchronous::continuation_task<cvpg::image_gray_8bit>("k_means_task::determine_cluster_task")
        , m_image(std::move(image))
        , m_centers(std::move(centers))
    {}

    void operator()()
    {
        auto tf = cvpg::imageproc::algorithms::tiling_functors::image<cvpg::image_rgb_8bit, cvpg::image_gray_8bit>({{ *m_image }});
        tf.parameters.image_width = m_image->width();
        tf.parameters.image_height = m_image->height();
        tf.parameters.cutoff_x = 512; // TODO use parameter
        tf.parameters.cutoff_y = 512; // TODO use parameter

        tf.tile_algorithm_task = [centers = m_centers](std::shared_ptr<cvpg::image_rgb_8bit> src1, std::shared_ptr<cvpg::image_rgb_8bit> /*src2*/, std::shared_ptr<cvpg::image_gray_8bit> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
        {
            determine_cluster_rgb_8bit(src1->data(0).get(), src1->data(1).get(), src1->data(2).get(), dst->data(0).get(), from_x, to_x, from_y, to_y, std::move(parameters), std::move(centers));
        };

        boost::asynchronous::create_callback_continuation(
            [result = this->this_task_result()](auto cont_res)
            {
                try
                {
                    result.set_value(std::get<0>(cont_res).get());
                }
                catch (...)
                {
                    result.set_exception(std::current_exception());
                }
            },
            cvpg::imageproc::algorithms::tiling(std::move(tf))
        );
    }

private:
    std::shared_ptr<cvpg::image_rgb_8bit> m_image;

    std::vector<point> m_centers;
};

template<class image_type>
boost::asynchronous::detail::callback_continuation<cvpg::image_gray_8bit> determine_cluster(std::shared_ptr<image_type> image, std::vector<point> centers)
{
    return boost::asynchronous::top_level_callback_continuation<cvpg::image_gray_8bit>(
               determine_cluster_task<image_type>(std::move(image), std::move(centers))
           );
}

template<class image_type>
struct calculate_cluster_means_task : public boost::asynchronous::continuation_task<std::pair<cvpg::image_gray_8bit, std::vector<point> > > {};

template<>
struct calculate_cluster_means_task<cvpg::image_gray_8bit> : public boost::asynchronous::continuation_task<std::pair<cvpg::image_gray_8bit, std::vector<point> > >
{
    calculate_cluster_means_task(std::shared_ptr<cvpg::image_gray_8bit> image, cvpg::image_gray_8bit clusters, std::size_t k)
        : boost::asynchronous::continuation_task<std::pair<cvpg::image_gray_8bit, std::vector<point> > >("k_means_task::calculate_cluster_means_task")
        , m_image(std::move(image))
        , m_clusters(std::make_shared<cvpg::image_gray_8bit>(std::move(clusters)))
        , m_k(k)
    {}

    void operator()()
    {
        auto tf = cvpg::imageproc::algorithms::tiling_functors::histogram<cvpg::image_gray_8bit, std::vector<cvpg::histogram<std::size_t> > >({{ *m_image }});
        tf.parameters.image_width = m_image->width();
        tf.parameters.image_height = m_image->height();
        tf.parameters.cutoff_x = 512; // TODO use parameter
        tf.parameters.cutoff_y = 512; // TODO use parameter

        tf.tile_algorithm_task = [clusters = m_clusters, k = m_k](std::shared_ptr<cvpg::image_gray_8bit> src1, std::shared_ptr<cvpg::image_gray_8bit> /*src2*/, std::shared_ptr<std::vector<cvpg::histogram<std::size_t> > > dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
        {
            // for each cluster ('k' pieces) create one histograms
            std::vector<std::vector<std::size_t> > h(k, std::vector<std::size_t>(256));

            for (std::size_t i = 0; i < k; ++i)
            {
                histogram_if_gray_8bit(src1->data(0).get(), &h[i], clusters->data(0).get(), [i](std::uint8_t const & c){ return c == (i + 1); }, from_x, to_x, from_y, to_y, parameters);
            }

            // move arrays into histogram structure
            std::vector<cvpg::histogram<std::size_t> > histograms;
            histograms.reserve(h.size());

            std::for_each(h.begin(),
                          h.end(),
                          [&histograms](auto & histogram)
                          {
                              histograms.emplace_back(std::move(histogram));
                          });

            *dst = std::move(histograms);
        };

        tf.horizontal_merge_task = [](std::shared_ptr<std::vector<cvpg::histogram<std::size_t> > > dst1, std::shared_ptr<std::vector<cvpg::histogram<std::size_t> > > dst2, std::size_t /*from_x*/, std::size_t /*to_x*/, std::size_t /*from_y*/, std::size_t /*to_y*/, cvpg::imageproc::algorithms::tiling_parameters /*parameters*/)
        {
            return merge_histograms(dst1, dst2);
        };

        tf.vertical_merge_task = [](std::shared_ptr<std::vector<cvpg::histogram<std::size_t> > > dst1, std::shared_ptr<std::vector<cvpg::histogram<std::size_t> > > dst2, std::size_t /*from_x*/, std::size_t /*to_x*/, std::size_t /*from_y*/, std::size_t /*to_y*/, cvpg::imageproc::algorithms::tiling_parameters /*parameters*/)
        {
            return merge_histograms(dst1, dst2);
        };

        boost::asynchronous::create_callback_continuation(
            [result = this->this_task_result(), image = std::move(m_image), clusters = std::move(m_clusters), k = m_k](auto cont_res) mutable
            {
                try
                {
                    auto histograms = std::move(std::get<0>(cont_res).get());

                    auto sum_histogram =
                        [](const auto & histogram) -> std::pair<std::size_t, std::size_t>
                        {
                            std::size_t sum = 0;
                            std::size_t entries = 0;

                            for (std::size_t i = 0; i < histogram.bins(); ++i)
                            {
                                sum += i * histogram.at(i);
                                entries += histogram.at(i);
                            }

                            return { sum, entries };
                        };

                    // create new mean centers of all clusters
                    std::vector<point> centers;
                    centers.reserve(k);

                    for (std::size_t i = 0; i < k; ++i)
                    {
                        auto [sum, entries] = sum_histogram(histograms.at(i));

                        centers.emplace_back(
                            entries != 0 ? (sum / entries) : 0,
                            0,
                            0
                        );
                    }

                    result.set_value({ std::move(*clusters), std::move(centers) });
                }
                catch (...)
                {
                    result.set_exception(std::current_exception());
                }
            },
            cvpg::imageproc::algorithms::tiling(std::move(tf))
        );
    }

private:
    std::shared_ptr<cvpg::image_gray_8bit> m_image;

    std::shared_ptr<cvpg::image_gray_8bit> m_clusters;

    std::size_t m_k;
};

template<>
struct calculate_cluster_means_task<cvpg::image_rgb_8bit> : public boost::asynchronous::continuation_task<std::pair<cvpg::image_gray_8bit, std::vector<point> > >
{
    calculate_cluster_means_task(std::shared_ptr<cvpg::image_rgb_8bit> image, cvpg::image_gray_8bit clusters, std::size_t k)
        : boost::asynchronous::continuation_task<std::pair<cvpg::image_gray_8bit, std::vector<point> > >("k_means_task::calculate_cluster_means_task")
        , m_image(std::move(image))
        , m_clusters(std::make_shared<cvpg::image_gray_8bit>(std::move(clusters)))
        , m_k(k)
    {}

    void operator()()
    {
        auto tf = cvpg::imageproc::algorithms::tiling_functors::histogram<cvpg::image_rgb_8bit, std::vector<cvpg::histogram<std::size_t> > >({{ *m_image }});
        tf.parameters.image_width = m_image->width();
        tf.parameters.image_height = m_image->height();
        tf.parameters.cutoff_x = 512; // TODO use parameter
        tf.parameters.cutoff_y = 512; // TODO use parameter

        tf.tile_algorithm_task = [clusters = m_clusters, k = m_k](std::shared_ptr<cvpg::image_rgb_8bit> src1, std::shared_ptr<cvpg::image_rgb_8bit> /*src2*/, std::shared_ptr<std::vector<cvpg::histogram<std::size_t> > > dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
        {
            // for each cluster ('k' pieces) create 3 histograms for red, green and blue parts
            std::vector<std::vector<std::size_t> > h(k * 3, std::vector<std::size_t>(256));

            for (std::size_t i = 0; i < k; ++i)
            {
                histogram_if_gray_8bit(src1->data(0).get(), &h[i * 3 + 0], clusters->data(0).get(), [i](std::uint8_t const & c){ return c == (i + 1); }, from_x, to_x, from_y, to_y, parameters);
                histogram_if_gray_8bit(src1->data(1).get(), &h[i * 3 + 1], clusters->data(0).get(), [i](std::uint8_t const & c){ return c == (i + 1); }, from_x, to_x, from_y, to_y, parameters);
                histogram_if_gray_8bit(src1->data(2).get(), &h[i * 3 + 2], clusters->data(0).get(), [i](std::uint8_t const & c){ return c == (i + 1); }, from_x, to_x, from_y, to_y, parameters);
            }

            // move arrays into histogram structure
            std::vector<cvpg::histogram<std::size_t> > histograms;
            histograms.reserve(h.size());

            std::for_each(h.begin(),
                          h.end(),
                          [&histograms](auto & histogram)
                          {
                              histograms.emplace_back(std::move(histogram));
                          });

            *dst = std::move(histograms);
        };

        tf.horizontal_merge_task = [](std::shared_ptr<std::vector<cvpg::histogram<std::size_t> > > dst1, std::shared_ptr<std::vector<cvpg::histogram<std::size_t> > > dst2, std::size_t /*from_x*/, std::size_t /*to_x*/, std::size_t /*from_y*/, std::size_t /*to_y*/, cvpg::imageproc::algorithms::tiling_parameters /*parameters*/)
        {
            return merge_histograms(dst1, dst2);
        };

        tf.vertical_merge_task = [](std::shared_ptr<std::vector<cvpg::histogram<std::size_t> > > dst1, std::shared_ptr<std::vector<cvpg::histogram<std::size_t> > > dst2, std::size_t /*from_x*/, std::size_t /*to_x*/, std::size_t /*from_y*/, std::size_t /*to_y*/, cvpg::imageproc::algorithms::tiling_parameters /*parameters*/)
        {
            return merge_histograms(dst1, dst2);
        };

        boost::asynchronous::create_callback_continuation(
            [result = this->this_task_result(), image = std::move(m_image), clusters = std::move(m_clusters), k = m_k](auto cont_res) mutable
            {
                try
                {
                    auto histograms = std::move(std::get<0>(cont_res).get());

                    if (histograms.size() % 3 != 0)
                    {
                        throw cvpg::exception("invalid amount of histograms");
                    }

                    auto sum_histogram =
                        [](const auto & histogram) -> std::pair<std::size_t, std::size_t>
                        {
                            std::size_t sum = 0;
                            std::size_t entries = 0;

                            for (std::size_t i = 0; i < histogram.bins(); ++i)
                            {
                                sum += i * histogram.at(i);
                                entries += histogram.at(i);
                            }

                            return { sum, entries };
                        };

                    // create new mean centers of all clusters
                    std::vector<point> centers;
                    centers.reserve(k);

                    for (std::size_t i = 0; i < k; ++i)
                    {
                        auto [sum_red, entries_red] = sum_histogram(histograms.at(i * 3 + 0));
                        auto [sum_green, entries_green] = sum_histogram(histograms.at(i * 3 + 1));
                        auto [sum_blue, entries_blue] = sum_histogram(histograms.at(i * 3 + 2));

                        centers.emplace_back(
                            entries_red != 0 ? (sum_red / entries_red) : 0,
                            entries_green != 0 ? (sum_green / entries_green) : 0,
                            entries_blue != 0 ? (sum_blue / entries_blue) : 0
                        );
                    }

                    result.set_value({ std::move(*clusters), std::move(centers) });
                }
                catch (...)
                {
                    result.set_exception(std::current_exception());
                }
            },
            cvpg::imageproc::algorithms::tiling(std::move(tf))
        );
    }

private:
    std::shared_ptr<cvpg::image_rgb_8bit> m_image;

    std::shared_ptr<cvpg::image_gray_8bit> m_clusters;

    std::size_t m_k;
};

template<class image_type>
boost::asynchronous::detail::callback_continuation<std::pair<cvpg::image_gray_8bit, std::vector<point> > > calculate_cluster_means(std::shared_ptr<image_type> image, cvpg::image_gray_8bit clusters, std::size_t k)
{
    return boost::asynchronous::top_level_callback_continuation<std::pair<cvpg::image_gray_8bit, std::vector<point> > >(
               calculate_cluster_means_task<image_type>(std::move(image), std::move(clusters), k)
           );
}

void create_cluster_result_gray_8bit(std::uint8_t * src, std::uint8_t * dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters, std::vector<point> centers)
{
    const std::size_t image_width = parameters.image_width;

    std::uint8_t * src_line = nullptr;
    std::uint8_t * dst_line = nullptr;

    for (std::size_t y = from_y; y <= to_y; ++y)
    {
        const std::size_t offset_y = image_width * y;

        src_line = src + offset_y;
        dst_line = dst + offset_y;

        for (std::size_t x = from_x; x <= to_x; ++x)
        {
            const std::uint8_t cluster_number = src_line[x] - 1;
            const auto & cluster = centers.at(cluster_number);

            dst_line[x] = cluster.x;
        }
    }
}

void create_cluster_result_rgb_8bit(std::uint8_t * src, std::uint8_t * dst_red, std::uint8_t * dst_green, std::uint8_t * dst_blue, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters, std::vector<point> centers)
{
    const std::size_t image_width = parameters.image_width;

    std::uint8_t * src_line = nullptr;
    std::uint8_t * dst_red_line = nullptr;
    std::uint8_t * dst_green_line = nullptr;
    std::uint8_t * dst_blue_line = nullptr;

    for (std::size_t y = from_y; y <= to_y; ++y)
    {
        const std::size_t offset_y = image_width * y;

        src_line = src + offset_y;
        dst_red_line = dst_red + offset_y;
        dst_green_line = dst_green + offset_y;
        dst_blue_line = dst_blue + offset_y;

        for (std::size_t x = from_x; x <= to_x; ++x)
        {
            const std::uint8_t cluster_number = src_line[x] - 1;
            const auto & cluster = centers.at(cluster_number);

            dst_red_line[x] = cluster.x;
            dst_green_line[x] = cluster.y;
            dst_blue_line[x] = cluster.z;
        }
    }
}

template<class image_type>
struct create_result_image_task : public boost::asynchronous::continuation_task<image_type> {};

template<>
struct create_result_image_task<cvpg::image_gray_8bit> : public boost::asynchronous::continuation_task<cvpg::image_gray_8bit>
{
    create_result_image_task(std::shared_ptr<cvpg::image_gray_8bit> clusters, std::vector<point> centers)
        : boost::asynchronous::continuation_task<cvpg::image_gray_8bit>("k_means_task::create_result_image_task")
        , m_clusters(std::move(clusters))
        , m_centers(std::move(centers))
    {}

    void operator()()
    {
        auto result_image = std::make_shared<cvpg::image_gray_8bit>(m_clusters->width(), m_clusters->height());

        auto tf = cvpg::imageproc::algorithms::tiling_functors::image<cvpg::image_gray_8bit, cvpg::image_gray_8bit>({{ *m_clusters }});
        tf.parameters.image_width = m_clusters->width();
        tf.parameters.image_height = m_clusters->height();
        tf.parameters.cutoff_x = 512; // TODO use parameter
        tf.parameters.cutoff_y = 512; // TODO use parameter

        tf.tile_algorithm_task = [centers = m_centers](std::shared_ptr<cvpg::image_gray_8bit> src1, std::shared_ptr<cvpg::image_gray_8bit> /*src2*/, std::shared_ptr<cvpg::image_gray_8bit> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
        {
            create_cluster_result_gray_8bit(src1->data(0).get(), dst->data(0).get(), from_x, to_x, from_y, to_y, std::move(parameters), std::move(centers));
        };

        boost::asynchronous::create_callback_continuation(
            [result = this->this_task_result()](auto cont_res)
            {
                try
                {
                    result.set_value(std::get<0>(cont_res).get());
                }
                catch (...)
                {
                    result.set_exception(std::current_exception());
                }
            },
            cvpg::imageproc::algorithms::tiling(std::move(tf))
        );
    }

private:
    std::shared_ptr<cvpg::image_gray_8bit> m_clusters;

    std::vector<point> m_centers;
};

template<>
struct create_result_image_task<cvpg::image_rgb_8bit> : public boost::asynchronous::continuation_task<cvpg::image_rgb_8bit>
{
    create_result_image_task(std::shared_ptr<cvpg::image_gray_8bit> clusters, std::vector<point> centers)
        : boost::asynchronous::continuation_task<cvpg::image_rgb_8bit>("k_means_task::create_result_image_task")
        , m_clusters(std::move(clusters))
        , m_centers(std::move(centers))
    {}

    void operator()()
    {
        auto result_image = std::make_shared<cvpg::image_rgb_8bit>(m_clusters->width(), m_clusters->height());

        auto tf = cvpg::imageproc::algorithms::tiling_functors::image<cvpg::image_gray_8bit, cvpg::image_rgb_8bit>({{ *m_clusters }});
        tf.parameters.image_width = m_clusters->width();
        tf.parameters.image_height = m_clusters->height();
        tf.parameters.cutoff_x = 512; // TODO use parameter
        tf.parameters.cutoff_y = 512; // TODO use parameter

        tf.tile_algorithm_task = [centers = m_centers](std::shared_ptr<cvpg::image_gray_8bit> src1, std::shared_ptr<cvpg::image_gray_8bit> /*src2*/, std::shared_ptr<cvpg::image_rgb_8bit> dst, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, cvpg::imageproc::algorithms::tiling_parameters parameters)
        {
            create_cluster_result_rgb_8bit(src1->data(0).get(), dst->data(0).get(), dst->data(1).get(), dst->data(2).get(), from_x, to_x, from_y, to_y, std::move(parameters), std::move(centers));
        };

        boost::asynchronous::create_callback_continuation(
            [result = this->this_task_result()](auto cont_res)
            {
                try
                {
                    result.set_value(std::get<0>(cont_res).get());
                }
                catch (...)
                {
                    result.set_exception(std::current_exception());
                }
            },
            cvpg::imageproc::algorithms::tiling(std::move(tf))
        );
    }

private:
    std::shared_ptr<cvpg::image_gray_8bit> m_clusters;

    std::vector<point> m_centers;
};

template<class image_type>
boost::asynchronous::detail::callback_continuation<image_type> create_result_image(std::shared_ptr<cvpg::image_gray_8bit> clusters, std::vector<point> centers)
{
    return boost::asynchronous::top_level_callback_continuation<image_type>(
               create_result_image_task<image_type>(std::move(clusters), std::move(centers))
           );
}

template<class image_type>
struct k_means_iteration_task : public boost::asynchronous::continuation_task<image_type>
{
    k_means_iteration_task(std::shared_ptr<image_type> image, std::vector<point> centers, std::size_t k, std::size_t iteration, std::size_t max_iterations)
        : boost::asynchronous::continuation_task<image_type>(std::string("k_means_task::k_means_iteration_task#iteration").append(std::to_string(iteration)))
        , m_image(std::move(image))
        , m_centers(std::move(centers))
        , m_k(k)
        , m_iteration(iteration)
        , m_max_iterations(max_iterations)
    {}

    void operator()()
    {
        auto old_centers = m_centers;

        boost::asynchronous::create_callback_continuation(
            [result = this->this_task_result(), old_centers = std::move(old_centers), iteration = m_iteration, max_iterations = m_max_iterations, image = m_image, k = m_k](auto cont_res) mutable
            {
                try
                {
                    auto cluster_result = std::move(std::get<0>(cont_res).get());

                    auto clusters_image = std::make_shared<cvpg::image_gray_8bit>(std::move(cluster_result.first));
                    auto new_centers = std::move(cluster_result.second);

                    // calculate the distances of old and new center points
                    std::vector<std::uint8_t> distances;
                    distances.reserve(old_centers.size());

                    std::transform(old_centers.begin(),
                                   old_centers.end(),
                                   new_centers.begin(),
                                   std::back_inserter(distances),
                                   [](point const & a, point const & b)
                                   {
                                       const auto dx = static_cast<std::int16_t>(a.x) - static_cast<std::int16_t>(b.x);
                                       const auto dy = static_cast<std::int16_t>(a.y) - static_cast<std::int16_t>(b.y);
                                       const auto dz = static_cast<std::int16_t>(a.z) - static_cast<std::int16_t>(b.z);

                                       return static_cast<std::uint8_t>(std::sqrt(dx * dx + dy * dy + dz * dz));
                                   });

                    // check if all distances are not moving (moving below threshold)
                    const bool all_centers_steady = std::all_of(distances.begin(),
                                                                distances.end(),
                                                                [](auto const & distance)
                                                                {
                                                                    return distance < 5; // TODO define a parameter for this !!!
                                                                });

                    // check if no further iterations are needed
                    if ((iteration + 1) >= max_iterations || all_centers_steady)
                    {
                        boost::asynchronous::create_callback_continuation(
                            [result = std::move(result)](auto cont_res) mutable
                            {
                                try
                                {
                                    result.set_value(std::move(std::get<0>(cont_res).get()));
                                }
                                catch (...)
                                {
                                    result.set_exception(std::current_exception());
                                }
                            },
                            create_result_image<image_type>(clusters_image, std::move(new_centers))
                        );
                    }
                    else
                    {
                        boost::asynchronous::create_callback_continuation(
                            [result = std::move(result)](auto cont_res) mutable
                            {
                                try
                                {
                                    result.set_value(std::move(std::get<0>(cont_res).get()));
                                }
                                catch (...)
                                {
                                    result.set_exception(std::current_exception());
                                }
                            },
                            k_means_iteration_task(image, std::move(new_centers), k, iteration + 1, max_iterations)
                        );
                    }
                }
                catch (...)
                {
                    result.set_exception(std::current_exception());
                }
            },
            boost::asynchronous::then(
                determine_cluster<image_type>(m_image, std::move(m_centers)),
                [image = m_image, k = m_k](auto cont_res)
                {
                    return calculate_cluster_means<image_type>(image, std::move(cont_res.get()), k);
                }
            )
        );
    }

private:
    std::shared_ptr<image_type> m_image;

    std::vector<point> m_centers;

    std::size_t m_k;

    std::size_t m_iteration;
    std::size_t m_max_iterations;
};

template<class image_type>
struct k_means_task : public boost::asynchronous::continuation_task<image_type>
{
    k_means_task(image_type image, std::size_t k, std::size_t max_iterations)
        : boost::asynchronous::continuation_task<image_type>("k_means_task")
        , m_image(std::make_shared<image_type>(std::move(image)))
        , m_k(k)
        , m_max_iterations(max_iterations)
    {}

    void operator()()
    {
        // create K random numbers
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distribution(0, 255);

        std::vector<point> centers;
        centers.reserve(m_k);

        for (std::size_t i = 0; i < m_k; ++i)
        {
            centers.emplace_back(distribution(gen), distribution(gen), distribution(gen));
        }

        boost::asynchronous::create_callback_continuation(
            [result = this->this_task_result()](auto cont_res)
            {
                try
                {
                    result.set_value(std::move(std::get<0>(cont_res).get()));
                }
                catch (...)
                {
                    result.set_exception(std::current_exception());
                }
            },
            k_means_iteration_task(m_image, std::move(centers), m_k, 0, m_max_iterations)
        );
    }

private:
    std::shared_ptr<image_type> m_image;

    std::size_t m_k;

    std::size_t m_max_iterations;
};

}

namespace cvpg::imageproc::algorithms {

boost::asynchronous::detail::callback_continuation<image_gray_8bit> k_means(image_gray_8bit image, std::size_t k, std::size_t max_iterations)
{
    return boost::asynchronous::top_level_callback_continuation<image_gray_8bit>(
               k_means_task(std::move(image), k, max_iterations)
           );
}

boost::asynchronous::detail::callback_continuation<image_rgb_8bit> k_means(image_rgb_8bit image, std::size_t k, std::size_t max_iterations)
{
    return boost::asynchronous::top_level_callback_continuation<image_rgb_8bit>(
               k_means_task(std::move(image), k, max_iterations)
           );
}

} // namespace cvpg::imageproc::algoritms
