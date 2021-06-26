// Copyright (c) 2020-2021 Franz Alt
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <libcvpg/imageproc/algorithms/hog.hpp>

#include <cmath>
#include <cstdint>
#include <cstring>
#include <exception>

#include <boost/asynchronous/algorithm/then.hpp>

namespace {

template<class image_type>
cvpg::histogram<double> calc_hog_cell(std::shared_ptr<image_type> image, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, bool l1_normalilze = true);

template<>
cvpg::histogram<double> calc_hog_cell(std::shared_ptr<cvpg::image_gray_8bit> image, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, bool l1_normalize)
{
    cvpg::histogram<std::size_t> histogram(9);

    const std::uint8_t * raw = image->data(0).get();
    const std::size_t image_width = image->width();

    std::int16_t gx = 0;
    std::int16_t gy = 0;
    double mag = 0.0;
    double angle = 0.0;

    // correct from/to-values to calculate the gradients up to the image borders
    if (from_x > 1)
    {
        --from_x;
    }
    else
    {
        from_x = 1;
    }

    if (to_x < (image->width() - 2))
    {
        ++to_x;
    }
    else
    {
        to_x = image->width() - 2;
    }

    if (from_y > 1)
    {
        --from_y;
    }
    else
    {
        from_y = 1;
    }

    if (to_y < (image->height() - 2))
    {
        ++to_y;
    }
    else
    {
        to_y = image->height() - 2;
    }

    for (std::size_t y = from_y; y <= to_y; ++y)
    {
        for (std::size_t x = from_x; x <= to_x; ++x)
        {
            // calculate the x/y gradients
            gx = static_cast<std::int16_t>(raw[y * image_width + (x + 1)]) - static_cast<std::int16_t>(raw[y * image_width + (x - 1)]);
            gy = static_cast<std::int16_t>(raw[(y + 1) * image_width + x]) - static_cast<std::int16_t>(raw[(y - 1) * image_width + x]);

            // calculate the gradient magnitude
            mag = std::sqrt(static_cast<double>(gx * gx + gy * gy));

            // calculate the absolute gradient angle
            angle = std::fabs((gx == 0) ? 0.0 : atan(static_cast<double>(gy) / static_cast<double>(gx)) * 180.0 / M_PI);

            const std::size_t bin = static_cast<std::size_t>(std::floor(angle / 20.0));

            histogram.at(bin) += static_cast<decltype(histogram)::value_type>(mag);
        }
    }

    cvpg::histogram<double> normalized_histogram(9);

    if (l1_normalize)
    {
        // normalize histogram (L1 norm)
        double sum = 0;

        for (auto const & h : histogram)
        {
            sum += std::fabs(static_cast<double>(h));
        }

        for (std::size_t i = 0; i < histogram.bins(); ++i)
        {
            normalized_histogram.at(i) = static_cast<double>(histogram.at(i)) / sum;
        }
    }
    else
    {
        // normalize histogram (L2 norm)
        double sum = 0;

        for (auto const & h : histogram)
        {
            auto h_ = std::fabs(static_cast<double>(h));
            sum += h_ * h_;
        }

        sum = std::sqrt(sum);

        for (std::size_t i = 0; i < histogram.bins(); ++i)
        {
            normalized_histogram.at(i) = std::min(1.0, static_cast<double>(histogram.at(i)) / sum);
        }
    }

    return normalized_histogram;
}

template<>
cvpg::histogram<double> calc_hog_cell(std::shared_ptr<cvpg::image_rgb_8bit> image, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, bool l1_normalize)
{
    // TODO implement me!

    return cvpg::histogram<double>(9);
}

template<class image_type>
struct hog_col_task : public boost::asynchronous::continuation_task<std::vector<cvpg::histogram<double> > >
{
    hog_col_task(std::shared_ptr<image_type> image, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, std::size_t cell_dimension, std::size_t sequential_cells_per_row)
        : boost::asynchronous::continuation_task<std::vector<cvpg::histogram<double> > >("hog_col")
        , m_image(image)
        , m_from_x(from_x)
        , m_to_x(to_x)
        , m_from_y(from_y)
        , m_to_y(to_y)
        , m_cell_dimension(cell_dimension)
        , m_sequential_cells_per_row(sequential_cells_per_row)
    {}

    void operator()()
    {
        auto task_res = this->this_task_result();

        const std::size_t cells = (m_to_x - m_from_x + 1) / m_cell_dimension;

        if (cells <= m_sequential_cells_per_row)
        {
            std::vector<cvpg::histogram<double> > hogs;
            hogs.reserve(cells);

            for (std::size_t i = 0; i < cells; ++i)
            {
                hogs.push_back(calc_hog_cell(m_image, m_from_x + i * m_cell_dimension, m_from_x + (i + 1) * m_cell_dimension - 1, m_from_y, m_to_y));
            }

            task_res.set_value(std::move(hogs));
        }
        else
        {
            // determine middle of x-range dependet on the cell dimension
            const std::size_t x_half = m_from_x + (cells / 2) * m_cell_dimension;

            boost::asynchronous::create_callback_continuation(
                [task_res = std::move(task_res)](auto cont_res) mutable
                {
                    try
                    {
                        auto h = std::move(std::get<1>(cont_res).get());

                        std::vector<cvpg::histogram<double> > hogs(std::move(std::get<0>(cont_res).get()));
                        hogs.insert(hogs.end(), h.begin(), h.end());

                        task_res.set_value(std::move(hogs));
                    }
                    catch (...)
                    {
                        task_res.set_exception(std::current_exception());
                    }
                },
                hog_col_task<image_type>(m_image, m_from_x, x_half - 1, m_from_y, m_to_y, m_cell_dimension, m_sequential_cells_per_row),
                hog_col_task<image_type>(m_image, x_half, m_to_x, m_from_y, m_to_y, m_cell_dimension, m_sequential_cells_per_row)
            );
        }
    }

private:
    std::shared_ptr<image_type> m_image;

    std::size_t m_from_x;
    std::size_t m_to_x;
    std::size_t m_from_y;
    std::size_t m_to_y;

    std::size_t m_cell_dimension;

    std::size_t m_sequential_cells_per_row;
};

template<class image_type>
struct hog_row_task : public boost::asynchronous::continuation_task<std::vector<cvpg::histogram<double> > >
{
    hog_row_task(std::shared_ptr<image_type> image, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y, std::size_t cell_dimension, std::size_t sequential_cells_per_row)
        : boost::asynchronous::continuation_task<std::vector<cvpg::histogram<double> > >("hog_row")
        , m_image(image)
        , m_from_x(from_x)
        , m_to_x(to_x)
        , m_from_y(from_y)
        , m_to_y(to_y)
        , m_cell_dimension(cell_dimension)
        , m_sequential_cells_per_row(sequential_cells_per_row)
    {}

    void operator()()
    {
        auto task_res = this->this_task_result();

        const std::size_t cells = (m_to_y - m_from_y + 1) / m_cell_dimension;

        if (cells <= 1)
        {
            boost::asynchronous::create_callback_continuation(
                [task_res = std::move(task_res)](auto cont_res) mutable
                {
                    try
                    {
                        task_res.set_value(std::move(std::get<0>(cont_res).get()));
                    }
                    catch (...)
                    {
                        task_res.set_exception(std::current_exception());
                    }
                },
                hog_col_task<image_type>(m_image, m_from_x, m_to_x, m_from_y, m_to_y, m_cell_dimension, m_sequential_cells_per_row)
            );
        }
        else
        {
            // determine middle of y-range dependet on the cell dimension
            const std::size_t y_half = m_from_y + (cells / 2) * m_cell_dimension;

            boost::asynchronous::create_callback_continuation(
                [task_res = std::move(task_res)](auto cont_res) mutable
                {
                    try
                    {
                        auto h = std::move(std::get<1>(cont_res).get());

                        std::vector<cvpg::histogram<double> > hogs(std::move(std::get<0>(cont_res).get()));
                        hogs.insert(hogs.end(), h.begin(), h.end());

                        task_res.set_value(std::move(hogs));
                    }
                    catch (...)
                    {
                        task_res.set_exception(std::current_exception());
                    }
                },
                hog_row_task<image_type>(m_image, m_from_x, m_to_x, m_from_y, y_half - 1, m_cell_dimension, m_sequential_cells_per_row),
                hog_row_task<image_type>(m_image, m_from_x, m_to_x, y_half, m_to_y, m_cell_dimension, m_sequential_cells_per_row)
            );
        }
    }

private:
    std::shared_ptr<image_type> m_image;

    std::size_t m_from_x;
    std::size_t m_to_x;
    std::size_t m_from_y;
    std::size_t m_to_y;

    std::size_t m_cell_dimension;

    std::size_t m_sequential_cells_per_row;
};

template<class image_type>
struct hog_task : public boost::asynchronous::continuation_task<std::vector<cvpg::histogram<double> > >
{
    hog_task(image_type image, std::size_t cell_dimension, std::size_t sequential_cells_per_row = 4)
        : boost::asynchronous::continuation_task<std::vector<cvpg::histogram<double> > >("hog")
        , m_image(std::make_shared<image_type>(std::forward<image_type>(image)))
        , m_cell_dimension(cell_dimension)
        , m_sequential_cells_per_row(sequential_cells_per_row)
    {}

    void operator()()
    {
        auto task_res = this->this_task_result();

        // determine the amount of rows and columns dependent on the desired cell dimension
        const std::size_t cols = m_image->width() / m_cell_dimension;
        const std::size_t rows = m_image->height() / m_cell_dimension;

        boost::asynchronous::create_callback_continuation(
            [task_res = std::move(task_res)](auto cont_res) mutable
            {
                try
                {
                    task_res.set_value(std::move(std::get<0>(cont_res).get()));
                }
                catch (...)
                {
                    task_res.set_exception(std::current_exception());
                }
            },
            hog_row_task<image_type>(m_image, 0, m_image->width() - 1, 0, m_image->height() - 1, m_cell_dimension, m_sequential_cells_per_row)
        );
    }

private:
    std::shared_ptr<image_type> m_image;

    std::size_t m_cell_dimension;

    std::size_t m_sequential_cells_per_row;
};

void paint_hog_cell(std::shared_ptr<cvpg::image_gray_8bit> image, cvpg::histogram<double> const & histogram, std::size_t from_x, std::size_t to_x, std::size_t from_y, std::size_t to_y)
{
    auto raw = image->data(0).get();
    const std::size_t image_width = image->width();

    const std::size_t half_x = from_x + (to_x - from_x) / 2;
    const std::size_t half_y = from_y + (to_y - from_y) / 2;

    auto draw_line_from_center =
        [raw, image_width, h_x = half_x, h_y = half_y](std::int32_t dx, std::int32_t dy, double increment)
        {
            // calculate start point
            const std::int32_t x_s = h_x - abs(dx);
            const std::int32_t y_s = h_y - abs(dy);

            // calculate end point
            const std::int32_t x_e = h_x + abs(dx);
            const std::int32_t y_e = h_y + abs(dy);

            if (abs(dx) > abs(dy))
            {
                // calculate the slope of the line
                const std::int32_t s = dy / dx;

                std::size_t y = y_s;

                for (std::size_t x = x_s; x <= x_e; ++x, y += s)
                {
                    std::uint8_t * r = raw + y * image_width + x;

                    *r = std::min(
                            static_cast<std::uint8_t>(255),
                            static_cast<std::uint8_t>(*r + increment * 255.0)
                         );
                }
            }
            else
            {
                // calculate the slope of the line
                const std::int32_t s = dx / dy;

                std::size_t x = x_s;

                for (std::size_t y = y_s; y <= y_e; ++y, x += s)
                {
                    std::uint8_t * r = raw + y * image_width + x;

                    *r = std::min(
                            static_cast<std::uint8_t>(255),
                            static_cast<std::uint8_t>(*r + increment * 255.0)
                         );
                }
            }
        };

    for (std::size_t i = 0; i < histogram.bins(); ++i)
    {
        auto const & h = histogram.at(i);

        if (h != 0.0)
        {
            if (i == 0)
            {
                // case: angle is at range [0..20) degree
                draw_line_from_center(0, 2, h);
            }
            else if (i == 1)
            {
                // case: angle it at range [20..40) degree
                draw_line_from_center(1, 2, h);
            }
            else if (i == 2)
            {
                // case: angle it at range [40..60) degree
                draw_line_from_center(2, 2, h);
            }
            else if (i == 3)
            {
                // case: angle it at range [60..80) degree
                draw_line_from_center(2, 1, h);
            }
            else if (i == 4)
            {
                // case: angle it at range [80..100) degree
                draw_line_from_center(2, 0, h);
            }
            else if (i == 5)
            {
                // case: angle it at range [100..120) degree
                draw_line_from_center(2, -1, h);
            }
            else if (i == 6)
            {
                // case: angle it at range [120..140) degree
                draw_line_from_center(2, -2, h);
            }
            else if (i == 7)
            {
                // case: angle it at range [140..160) degree
                draw_line_from_center(1, -2, h);
            }
            else if (i == 8)
            {
                // case: angle it at range [160..180) degree
                draw_line_from_center(0, -2, h);
            }
        }
    }
}

struct hog_image_col_task : public boost::asynchronous::continuation_task<void>
{
    hog_image_col_task(std::shared_ptr<cvpg::image_gray_8bit> image, std::shared_ptr<std::vector<cvpg::histogram<double> > > histograms, std::size_t from_cell_col, std::size_t to_cell_col, std::size_t from_cell_row, std::size_t to_cell_row, std::size_t cell_dimension)
        : boost::asynchronous::continuation_task<void>("hog_image_col")
        , m_image(std::move(image))
        , m_histograms(std::move(histograms))
        , m_from_cell_col(from_cell_col)
        , m_to_cell_col(to_cell_col)
        , m_from_cell_row(from_cell_row)
        , m_to_cell_row(to_cell_row)
        , m_cell_dimension(cell_dimension)
    {}

    void operator()()
    {
        auto task_res = this->this_task_result();

        const std::size_t cells = m_to_cell_col - m_from_cell_col + 1;

        if (cells <= 1)
        {
            const std::size_t cells_per_row = m_image->width() / m_cell_dimension;

            // determine histogram for current cell
            cvpg::histogram<double> const & h = m_histograms->at(m_from_cell_row * cells_per_row + m_from_cell_col);

            const std::size_t from_x = m_from_cell_col * m_cell_dimension;
            const std::size_t to_x = from_x + m_cell_dimension - 1;
            const std::size_t from_y = m_from_cell_row * m_cell_dimension;
            const std::size_t to_y = from_y + m_cell_dimension - 1;

            paint_hog_cell(m_image, h, from_x, to_x, from_y, to_y);

            task_res.set_value();
        }
        else
        {
            // determine middle of col-range
            const std::size_t col_half = m_from_cell_col + (cells / 2);

            boost::asynchronous::create_callback_continuation(
                [task_res = std::move(task_res)](auto cont_res) mutable
                {
                    try
                    {
                        std::get<0>(cont_res).get();
                        std::get<1>(cont_res).get();

                        task_res.set_value();
                    }
                    catch (...)
                    {
                        task_res.set_exception(std::current_exception());
                    }
                },
                hog_image_col_task(m_image, m_histograms, m_from_cell_col, col_half - 1, m_from_cell_row, m_to_cell_row, m_cell_dimension),
                hog_image_col_task(m_image, m_histograms, col_half, m_to_cell_col, m_from_cell_row, m_to_cell_row, m_cell_dimension)
            );
        }
    }

private:
    std::shared_ptr<cvpg::image_gray_8bit> m_image;

    std::shared_ptr<std::vector<cvpg::histogram<double> > > m_histograms;

    std::size_t m_from_cell_col;
    std::size_t m_to_cell_col;
    std::size_t m_from_cell_row;
    std::size_t m_to_cell_row;

    std::size_t m_cell_dimension;
};

struct hog_image_row_task : public boost::asynchronous::continuation_task<void>
{
    hog_image_row_task(std::shared_ptr<cvpg::image_gray_8bit> image, std::shared_ptr<std::vector<cvpg::histogram<double> > > histograms, std::size_t from_cell_col, std::size_t to_cell_col, std::size_t from_cell_row, std::size_t to_cell_row, std::size_t cell_dimension)
        : boost::asynchronous::continuation_task<void>("hog_image_row")
        , m_image(std::move(image))
        , m_histograms(std::move(histograms))
        , m_from_cell_col(from_cell_col)
        , m_to_cell_col(to_cell_col)
        , m_from_cell_row(from_cell_row)
        , m_to_cell_row(to_cell_row)
        , m_cell_dimension(cell_dimension)
    {}

    void operator()()
    {
        auto task_res = this->this_task_result();

        const std::size_t cells = m_to_cell_row - m_from_cell_row + 1;

        if (cells <= 1)
        {
            boost::asynchronous::create_callback_continuation(
                [task_res = std::move(task_res)](auto cont_res) mutable
                {
                    try
                    {
                        std::get<0>(cont_res).get();

                        task_res.set_value();
                    }
                    catch (...)
                    {
                        task_res.set_exception(std::current_exception());
                    }
                },
                hog_image_col_task(m_image, m_histograms, m_from_cell_col, m_to_cell_col, m_from_cell_row, m_to_cell_row, m_cell_dimension)
            );
        }
        else
        {
            // determine middle of row-range
            const std::size_t row_half = m_from_cell_row + (cells / 2);

            boost::asynchronous::create_callback_continuation(
                [task_res = std::move(task_res)](auto cont_res) mutable
                {
                    try
                    {
                        std::get<0>(cont_res).get();
                        std::get<1>(cont_res).get();

                        task_res.set_value();
                    }
                    catch (...)
                    {
                        task_res.set_exception(std::current_exception());
                    }
                },
                hog_image_row_task(m_image, m_histograms, m_from_cell_col, m_to_cell_col, m_from_cell_row, row_half - 1, m_cell_dimension),
                hog_image_row_task(m_image, m_histograms, m_from_cell_col, m_to_cell_col, row_half, m_to_cell_row, m_cell_dimension)
            );
        }
    }

private:
    std::shared_ptr<cvpg::image_gray_8bit> m_image;

    std::shared_ptr<std::vector<cvpg::histogram<double> > > m_histograms;

    std::size_t m_from_cell_col;
    std::size_t m_to_cell_col;
    std::size_t m_from_cell_row;
    std::size_t m_to_cell_row;

    std::size_t m_cell_dimension;
};

struct hog_image_task : public boost::asynchronous::continuation_task<cvpg::image_gray_8bit>
{
    hog_image_task(std::vector<cvpg::histogram<double> > histograms, std::size_t cells_per_row, std::size_t cell_dimension)
        : boost::asynchronous::continuation_task<cvpg::image_gray_8bit>("hog_image")
        , m_histograms(std::make_shared<std::vector<cvpg::histogram<double> > >(std::move(histograms)))
        , m_cells_per_row(cells_per_row)
        , m_cell_dimension(cell_dimension)
    {}

    void operator()()
    {
        auto task_res = this->this_task_result();

        const std::size_t cell_cols = m_cells_per_row;
        const std::size_t cell_rows = m_histograms->size() / cell_cols;

        // create a result image ...
        auto image = std::make_shared<cvpg::image_gray_8bit>(cell_cols * m_cell_dimension, cell_rows * m_cell_dimension);

        // ... and paint it black
        memset(image->data(0).get(), 0, image->width() * image->height());

        boost::asynchronous::create_callback_continuation(
            [task_res = std::move(task_res), image](auto cont_res) mutable
            {
                try
                {
                    std::get<0>(cont_res).get();

                    task_res.set_value(std::move(*image));
                }
                catch (...)
                {
                    task_res.set_exception(std::current_exception());
                }
            },
            hog_image_row_task(image, m_histograms, 0, cell_cols - 1, 0, cell_rows - 1, m_cell_dimension)
        );
    }

private:
    std::shared_ptr<std::vector<cvpg::histogram<double> > > m_histograms;

    std::size_t m_cells_per_row;

    std::size_t m_cell_dimension;
};

}

namespace cvpg::imageproc::algorithms {

boost::asynchronous::detail::callback_continuation<std::vector<histogram<double> > > hog(image_gray_8bit image, std::size_t cell_dimension)
{
    return boost::asynchronous::top_level_callback_continuation<std::vector<histogram<double> > >(
               hog_task(std::move(image), cell_dimension)
           );
}

boost::asynchronous::detail::callback_continuation<std::vector<histogram<double> > > hog(image_rgb_8bit image, std::size_t cell_dimension)
{
    return boost::asynchronous::top_level_callback_continuation<std::vector<histogram<double> > >(
               hog_task(std::move(image), cell_dimension)
           );
}

boost::asynchronous::detail::callback_continuation<image_gray_8bit> hog_image(std::vector<histogram<double> > histograms, std::size_t cells_per_row, std::size_t cell_dimension)
{
    return boost::asynchronous::top_level_callback_continuation<image_gray_8bit>(
               hog_image_task(std::move(histograms), cells_per_row, cell_dimension)
           );
}

boost::asynchronous::detail::callback_continuation<image_gray_8bit> hog_image(image_gray_8bit image)
{
    const std::size_t cell_dimension = 8;

    auto cells_per_row = image.width() / cell_dimension;

    return boost::asynchronous::then(
             hog(std::move(image), cell_dimension),
             [cells_per_row, cell_dimension](auto cont_res)
             {
                 return hog_image(std::move(cont_res.get()), cells_per_row, cell_dimension);
             }
           );
}

boost::asynchronous::detail::callback_continuation<image_gray_8bit> hog_image(image_rgb_8bit image)
{
    const std::size_t cell_dimension = 8;

    auto cells_per_row = image.width() / cell_dimension;

    return boost::asynchronous::then(
             hog(std::move(image), cell_dimension),
             [cells_per_row, cell_dimension](auto cont_res)
             {
                 return hog_image(std::move(cont_res.get()), cells_per_row, cell_dimension);
             }
           );
}

} // namespace cvpg::imageproc::algorithms
