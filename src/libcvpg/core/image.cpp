#include <libcvpg/core/image.hpp>

#include <stdio.h>
#include <string.h>
#include <png.h>

#include <libcvpg/core/exception.hpp>

namespace cvpg {

image_gray_8bit read_gray_8bit_png(std::string const & filename)
{
    int width = 0;
    int height = 0;
    png_byte color_type;
    png_byte bit_depth;
    png_bytep * row_pointers = nullptr;

    FILE * fp = fopen(filename.c_str(), "rb");

    if (!fp)
    {
        throw io_exception("cannot open file");
    }

    char header[8];    // 8 is the maximum size that can be checked

    if (fread(header, 1, sizeof(header), fp) < 8)
    {
        fclose(fp);

        throw io_exception("file is not a PNG image");
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png)
    {
        fclose(fp);

        throw io_exception("file is not a PNG image");
    }

    png_infop info = png_create_info_struct(png);

    if (!info)
    {
        fclose(fp);

        throw io_exception("file is not a PNG image");
    }

    if (setjmp(png_jmpbuf(png)))
    {
        fclose(fp);

        throw io_exception("file is not a PNG image");
    }

    png_init_io(png, fp);

    png_set_sig_bytes(png, sizeof(header));

    png_read_info(png, info);

    width      = png_get_image_width(png, info);
    height     = png_get_image_height(png, info);
    color_type = png_get_color_type(png, info);
    bit_depth  = png_get_bit_depth(png, info);

    if (bit_depth != 8)
    {
        fclose(fp);

        throw io_exception("invalid bit depth");
    }

    if (color_type != PNG_COLOR_TYPE_GRAY)
    {
        fclose(fp);

        throw io_exception("invalid color type");
    }

    if (png_get_valid(png, info, PNG_INFO_tRNS))
    {
//        png_set_tRNS_to_alpha(png);
    }

    png_read_update_info(png, info);

    row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);

    for (int y = 0; y < height; ++y)
    {
        row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png, info));
    }

    png_read_image(png, row_pointers);

    fclose(fp);

    std::shared_ptr<std::uint8_t> data(static_cast<std::uint8_t *>(malloc(width * height * sizeof(std::uint8_t))), [](std::uint8_t * ptr){ free(ptr); });

    image_gray_8bit img(width, height, 0, std::array<std::shared_ptr<std::uint8_t>, 1>({ data }));

    for (int y = 0; y < height; ++y)
    {
        memcpy(&img.data(0).get()[y * width * sizeof(std::uint8_t)], row_pointers[y], width * sizeof(std::uint8_t));
    }

    free(row_pointers);

    return img;
}

image_rgb_8bit read_rgb_8bit_png(std::string const & filename)
{
    int width = 0;
    int height = 0;
    png_byte color_type;
    png_byte bit_depth;
    png_bytep * row_pointers = nullptr;

    FILE * fp = fopen(filename.c_str(), "rb");

    if (!fp)
    {
        throw io_exception("cannot open file");
    }

    char header[8];    // 8 is the maximum size that can be checked

    if (fread(header, 1, sizeof(header), fp) < 8)
    {
        fclose(fp);

        throw io_exception("file is not a PNG image");
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png)
    {
        fclose(fp);

        throw io_exception("file is not a PNG image");
    }

    png_infop info = png_create_info_struct(png);

    if (!info)
    {
        fclose(fp);

        throw io_exception("file is not a PNG image");
    }

    if (setjmp(png_jmpbuf(png)))
    {
        fclose(fp);

        throw io_exception("file is not a PNG image");
    }

    png_init_io(png, fp);

    png_set_sig_bytes(png, sizeof(header));

    png_read_info(png, info);

    width      = png_get_image_width(png, info);
    height     = png_get_image_height(png, info);
    color_type = png_get_color_type(png, info);
    bit_depth  = png_get_bit_depth(png, info);

    if (bit_depth != 8)
    {
        fclose(fp);

        throw io_exception("invalid bit depth");
    }

    if (color_type != PNG_COLOR_TYPE_RGB)
    {
        fclose(fp);

        throw io_exception("invalid color type");
    }

    if (png_get_valid(png, info, PNG_INFO_tRNS))
    {
//        png_set_tRNS_to_alpha(png);
    }

    png_read_update_info(png, info);

    row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);

    for (int y = 0; y < height; ++y)
    {
        row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png, info));
    }

    png_read_image(png, row_pointers);

    fclose(fp);

    std::shared_ptr<std::uint8_t> data_r(static_cast<std::uint8_t *>(malloc(width * height * sizeof(std::uint8_t))), [](std::uint8_t * ptr){ free(ptr); });
    std::shared_ptr<std::uint8_t> data_g(static_cast<std::uint8_t *>(malloc(width * height * sizeof(std::uint8_t))), [](std::uint8_t * ptr){ free(ptr); });
    std::shared_ptr<std::uint8_t> data_b(static_cast<std::uint8_t *>(malloc(width * height * sizeof(std::uint8_t))), [](std::uint8_t * ptr){ free(ptr); });

    image_rgb_8bit img(width, height, 0, std::array<std::shared_ptr<std::uint8_t>, 3>({ data_r, data_g, data_b }));

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            img.data(0).get()[y * width * sizeof(std::uint8_t) + x] = row_pointers[y][x * 3];
            img.data(1).get()[y * width * sizeof(std::uint8_t) + x] = row_pointers[y][x * 3 + 1];
            img.data(2).get()[y * width * sizeof(std::uint8_t) + x] = row_pointers[y][x * 3 + 2];
        }
    }

    free(row_pointers);

    return img;
}

std::tuple<std::uint8_t, std::any> read_png(std::string const & filename)
{
    int width = 0;
    int height = 0;
    png_byte color_type;
    png_byte bit_depth;
    png_bytep * row_pointers = nullptr;

    FILE * fp = fopen(filename.c_str(), "rb");

    if (!fp)
    {
        throw io_exception("cannot open file");
    }

    char header[8];    // 8 is the maximum size that can be checked

    if (fread(header, 1, sizeof(header), fp) < 8)
    {
        fclose(fp);

        throw io_exception("file is not a PNG image");
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png)
    {
        fclose(fp);

        throw io_exception("file is not a PNG image");
    }

    png_infop info = png_create_info_struct(png);

    if (!info)
    {
        fclose(fp);

        throw io_exception("file is not a PNG image");
    }

    if (setjmp(png_jmpbuf(png)))
    {
        fclose(fp);

        throw io_exception("file is not a PNG image");
    }

    png_init_io(png, fp);

    png_set_sig_bytes(png, sizeof(header));

    png_read_info(png, info);

    width      = png_get_image_width(png, info);
    height     = png_get_image_height(png, info);
    color_type = png_get_color_type(png, info);
    bit_depth  = png_get_bit_depth(png, info);

    if (bit_depth != 8)
    {
        fclose(fp);

        throw io_exception("invalid bit depth");
    }

    std::uint8_t channels = 0;

    if (color_type == PNG_COLOR_TYPE_GRAY)
    {
        channels = 1;
    }
    else if (color_type == PNG_COLOR_TYPE_RGB)
    {
        channels = 3;
    }
    else
    {
        fclose(fp);

        throw io_exception("invalid color type (only grayscale or RGB images are supported)");
    }

    if (png_get_valid(png, info, PNG_INFO_tRNS))
    {
//        png_set_tRNS_to_alpha(png);
    }

    png_read_update_info(png, info);

    row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);

    for (int y = 0; y < height; ++y)
    {
        row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png, info));
    }

    png_read_image(png, row_pointers);

    fclose(fp);

    std::any image;

    if (channels == 1)
    {
        std::shared_ptr<std::uint8_t> data(static_cast<std::uint8_t *>(malloc(width * height * sizeof(std::uint8_t))), [](std::uint8_t * ptr){ free(ptr); });

        image_gray_8bit img(width, height, 0, std::array<std::shared_ptr<std::uint8_t>, 1>({ data }));

        for (int y = 0; y < height; ++y)
        {
            memcpy(&img.data(0).get()[y * width * sizeof(std::uint8_t)], row_pointers[y], width * sizeof(std::uint8_t));
        }

        image = std::any(std::move(img));
    }
    else // if (channels == 3)
    {
        std::shared_ptr<std::uint8_t> data_r(static_cast<std::uint8_t *>(malloc(width * height * sizeof(std::uint8_t))), [](std::uint8_t * ptr){ free(ptr); });
        std::shared_ptr<std::uint8_t> data_g(static_cast<std::uint8_t *>(malloc(width * height * sizeof(std::uint8_t))), [](std::uint8_t * ptr){ free(ptr); });
        std::shared_ptr<std::uint8_t> data_b(static_cast<std::uint8_t *>(malloc(width * height * sizeof(std::uint8_t))), [](std::uint8_t * ptr){ free(ptr); });

        image_rgb_8bit img(width, height, 0, std::array<std::shared_ptr<std::uint8_t>, 3>({ data_r, data_g, data_b }));

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                img.data(0).get()[y * width * sizeof(std::uint8_t) + x] = row_pointers[y][x * 3];
                img.data(1).get()[y * width * sizeof(std::uint8_t) + x] = row_pointers[y][x * 3 + 1];
                img.data(2).get()[y * width * sizeof(std::uint8_t) + x] = row_pointers[y][x * 3 + 2];
            }
        }

        image = std::any(std::move(img));
    }

    free(row_pointers);

    return { channels, std::move(image) };
}

void write_png(image_gray_8bit const & img, std::string const & filename)
{
    png_byte color_type = PNG_COLOR_TYPE_GRAY;
    png_byte bit_depth = 8;

    png_structp png_ptr = nullptr;
    png_infop info_ptr = nullptr;
    png_bytep * row_pointers = nullptr;

    FILE * fp = fopen(filename.c_str(), "wb");

    if (!fp)
    {
        throw io_exception("cannot open file");
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
    {
        fclose(fp);

        throw io_exception("cannot write PNG header");
    }

    info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr)
    {
        fclose(fp);

        throw io_exception("cannot write PNG header");
    }

    png_set_IHDR(png_ptr, info_ptr, img.width(), img.height(),
                 bit_depth, color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * img.height());

    for (int y = 0; y < img.height(); ++y)
    {
        row_pointers[y] = (png_byte*)malloc(/*png_get_rowbytes(png_ptr, info_ptr)*/img.width() * sizeof(png_byte));
        row_pointers[y] = img.data(0).get() + y * img.width();
    }

    png_init_io(png_ptr, fp);
    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, 1, NULL);

    free(row_pointers);
    fclose(fp);
}

void write_png(image_rgb_8bit const & img, std::string const & filename)
{
    png_byte color_type = PNG_COLOR_TYPE_RGB;
    png_byte bit_depth = 8;

    png_structp png_ptr = nullptr;
    png_infop info_ptr = nullptr;
    png_bytep * row_pointers = nullptr;

    FILE * fp = fopen(filename.c_str(), "wb");

    if (!fp)
    {
        throw io_exception("cannot open file");
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
    {
        fclose(fp);

        throw io_exception("cannot write PNG header");
    }

    info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr)
    {
        fclose(fp);

        throw io_exception("cannot write PNG header");
    }

    png_set_IHDR(png_ptr, info_ptr, img.width(), img.height(),
                 bit_depth, color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * img.height());

    for (int y = 0; y < img.height(); ++y)
    {
        row_pointers[y] = (png_byte*)malloc(/*png_get_rowbytes(png_ptr, info_ptr)*/img.width() * sizeof(png_byte) * 3);

        for (int x = 0; x < img.width(); ++x)
        {
            row_pointers[y][x * 3] = img.data(0).get()[y * img.width() * sizeof(std::uint8_t) + x];
            row_pointers[y][x * 3 + 1] = img.data(1).get()[y * img.width() * sizeof(std::uint8_t) + x];
            row_pointers[y][x * 3 + 2] = img.data(2).get()[y * img.width() * sizeof(std::uint8_t) + x];
        }
    }

    png_init_io(png_ptr, fp);
    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, 1, NULL);

    free(row_pointers);
    fclose(fp);
}

// manual instantation of image<> for some types
template class image<std::uint8_t, 1>;
template class image<std::uint8_t, 3>;

} // namespace cvpg
