//
// Created by Deamon on 9/8/2020.
//
#include "screenshotMaker.h"

void saveScreenshot(const std::string &name, int width, int height, std::vector <uint8_t> &rgbaBuff) {
    FILE *fp = fopen(name.c_str(), "wb");
    if (!fp) {
        return;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr) {
        fclose(fp);
        return;
    }

    png_infop png_info;
    if (!(png_info = png_create_info_struct(png_ptr))) {
        png_destroy_write_struct(&png_ptr, nullptr);
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, nullptr);
        return;
    }

    png_init_io(png_ptr, fp);

    png_set_IHDR(png_ptr, png_info, width, height, 8, PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    std::shared_ptr<std::vector<uint8_t>> data = std::make_shared<std::vector<uint8_t>>(width*height*3);
    std::shared_ptr<std::vector<uint8_t*>> rows = std::make_shared<std::vector<uint8_t*>>(height);

    for (int i = 0; i < height; ++i) {
        (*rows)[height - i - 1] = data->data() + (i*width*3);
        for (int j = 0; j < width; ++j) {
            int i1 = (i*width+j)*3;
            int i2 = (i*width+j)*4;

            char r = rgbaBuff[++i2];
            char g = rgbaBuff[++i2];
            char b = rgbaBuff[++i2];
            char a = rgbaBuff[++i2];

            (*data)[i1++] = a;
            (*data)[i1++] = r;
            (*data)[i1++] = g;
        }
    }
    png_set_rows(png_ptr, png_info, rows->data());
    png_write_png(png_ptr, png_info, PNG_TRANSFORM_IDENTITY, nullptr);
    png_write_end(png_ptr, png_info);


    png_destroy_write_struct(&png_ptr, nullptr);
    fclose(fp);
}

void saveDataFromDrawStage(const HFrameBuffer& fb,
                           const std::string& screenshotFileName,
                           int screenshotWidth, int screenshotHeight,
                           std::vector <uint8_t> &buffer) {
    if (fb == nullptr)
        return;

    fb->readRGBAPixels( 0, 0, screenshotWidth, screenshotHeight, buffer.data());
    saveScreenshot(screenshotFileName, screenshotWidth, screenshotHeight, buffer);
}
