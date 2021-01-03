//
// Created by Deamon on 9/8/2020.
//
#include "screenshotMaker.h"
#include "lodepng/lodepng.h"
#include <fstream>
#ifndef __EMSCRIPTEN__
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
        (*rows)[i] = data->data() + (i*width*3);
        for (int j = 0; j < width; ++j) {
            int i1 = (i*width+j)*3;
            int i2 = (i*width+j)*4;

            char r = rgbaBuff[i2++];
            char g = rgbaBuff[i2++];
            char b = rgbaBuff[i2++];
            char a = rgbaBuff[i2++];

            (*data)[i1++] = r;
            (*data)[i1++] = g;
            (*data)[i1++] = b;
        }
    }
    png_set_rows(png_ptr, png_info, rows->data());
    png_write_png(png_ptr, png_info, PNG_TRANSFORM_IDENTITY, nullptr);
    png_write_end(png_ptr, png_info);


    png_destroy_write_struct(&png_ptr, nullptr);
    fclose(fp);
}
#endif

//Lodepng part
void saveScreenshotLodePng(const std::string &name, int width, int height, std::vector <uint8_t> &rgbaBuff){
    unsigned char* png;
    size_t pngsize;

    unsigned error = lodepng_encode32(&png, &pngsize, rgbaBuff.data(), width, height);
    if(!error) {
        std::ofstream f(name);
        f.write((const char *)(png), pngsize);
    }

    /*if there's an error, display it*/
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

    free(png);
}



//BMP part
const int bytesPerPixel = 4; /// red, green, blue
const int fileHeaderSize = 14;
const int infoHeaderSize = 52;
unsigned char* createBitmapFileHeader(int height, int width, int pitch, int paddingSize);
unsigned char* createBitmapInfoHeader (int height, int width);

void generateBitmapImage(unsigned char *image, int height, int width, int pitch, const char* imageFileName) {

    unsigned char padding[3] = { 0, 0, 0 };
    int paddingSize = (4 - (/*width*bytesPerPixel*/ pitch) % 4) % 4;

    unsigned char* fileHeader = createBitmapFileHeader(height, width, pitch, paddingSize);
    unsigned char* infoHeader = createBitmapInfoHeader(height, width);

    FILE* imageFile = fopen(imageFileName, "wb");

    fwrite(fileHeader, 1, fileHeaderSize, imageFile);
    fwrite(infoHeader, 1, infoHeaderSize, imageFile);

    int i;
    for (i = 0; i < height; i++) {
        fwrite(image + (i*pitch /*width*bytesPerPixel*/), bytesPerPixel, width, imageFile);
        fwrite(padding, 1, paddingSize, imageFile);
    }

    fclose(imageFile);
    //free(fileHeader);
    //free(infoHeader);
}

unsigned char* createBitmapFileHeader(int height, int width, int pitch, int paddingSize) {
    int fileSize = fileHeaderSize + infoHeaderSize + (/*bytesPerPixel*width*/pitch + paddingSize) * height;

    static unsigned char fileHeader[] = {
        0,0, /// signature
        0,0,0,0, /// image file size in bytes
        0,0,0,0, /// reserved
        0,0,0,0, /// start of pixel array
    };

    fileHeader[0] = (unsigned char)('B');
    fileHeader[1] = (unsigned char)('M');
    fileHeader[2] = (unsigned char)(fileSize);
    fileHeader[3] = (unsigned char)(fileSize >> 8);
    fileHeader[4] = (unsigned char)(fileSize >> 16);
    fileHeader[5] = (unsigned char)(fileSize >> 24);
    fileHeader[10] = (unsigned char)(fileHeaderSize + infoHeaderSize);

    return fileHeader;
}

unsigned char* createBitmapInfoHeader(int height, int width) {
    static unsigned char infoHeader[infoHeaderSize] = {
        0,0,0,0, /// header size
        0,0,0,0, /// image width
        0,0,0,0, /// image height
        0,0, /// number of color planes
        0,0, /// bits per pixel
        03,0,0,0, /// compression
        0,0,0,0, /// image size
        0,0,0,0, /// horizontal resolution
        0,0,0,0, /// vertical resolution
        0,0,0,0, /// colors in color table
        0,0,0,0, /// important color count
        0xFF,00,00,00,
        00,0xFF,00,00,
        00,00,0xFF,00,
    };

    infoHeader[0] = (unsigned char)(infoHeaderSize);
    infoHeader[4] = (unsigned char)(width);
    infoHeader[5] = (unsigned char)(width >> 8);
    infoHeader[6] = (unsigned char)(width >> 16);
    infoHeader[7] = (unsigned char)(width >> 24);
    infoHeader[8] = (unsigned char)(height);
    infoHeader[9] = (unsigned char)(height >> 8);
    infoHeader[10] = (unsigned char)(height >> 16);
    infoHeader[11] = (unsigned char)(height >> 24);
    infoHeader[12] = (unsigned char)(1);
    infoHeader[14] = (unsigned char)(bytesPerPixel * 8);

    return infoHeader;
}

void saveDataFromDrawStage(const HFrameBuffer& fb,
                           const std::string& screenshotFileName,
                           int screenshotWidth, int screenshotHeight,
                           std::vector <uint8_t> &buffer) {
    if (fb == nullptr)
        return;

    std::vector <uint8_t> internalBuffer = std::vector<uint8_t>(screenshotWidth*screenshotHeight*4);
    fb->readRGBAPixels( 0, 0, screenshotWidth, screenshotHeight, internalBuffer.data());

    //In internalbuffer rn image is in bgra format. Let's turn it into RGBA and make alpha 256
    for (int j = 0; j < screenshotHeight; j++) {
        for (int i = 0; i < screenshotWidth; i++) {
            std::array<int,4> ind =
            {
                screenshotWidth * 4 * (screenshotHeight - 1 - j) + (i * 4) + 0,
                screenshotWidth * 4 * (screenshotHeight - 1 - j) + (i * 4) + 1,
                screenshotWidth * 4 * (screenshotHeight - 1 - j) + (i * 4) + 2,
                screenshotWidth * 4 * (screenshotHeight - 1 - j) + (i * 4) + 3,
            };
            std::array<int,4> ind2 =
                {
                    screenshotWidth * 4 * (j) + (i * 4) + 0,
                    screenshotWidth * 4 * (j) + (i * 4) + 1,
                    screenshotWidth * 4 * (j) + (i * 4) + 2,
                    screenshotWidth * 4 * (j) + (i * 4) + 3,
                };


            auto b = internalBuffer[ind[0]];
            auto g = internalBuffer[ind[1]];
            auto r = internalBuffer[ind[2]];
            auto a = internalBuffer[ind[3]];

            buffer[ind2[0]] = b;
            buffer[ind2[1]] = g;
            buffer[ind2[2]] = r;
            buffer[ind2[3]] = 255;
        }
    }

//    generateBitmapImage((unsigned char*) buffer.data(), screenshotHeight, screenshotWidth, screenshotWidth*4, (screenshotFileName+".bmp").c_str());
    //saveScreenshot(screenshotFileName, screenshotWidth, screenshotHeight, buffer);
    saveScreenshotLodePng(screenshotFileName, screenshotWidth, screenshotHeight, buffer);

}
