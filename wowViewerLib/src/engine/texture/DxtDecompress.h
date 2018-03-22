//
// Created by Deamon on 3/22/2018.
//

#ifndef WEBWOWVIEWERCPP_DXTDECOMPRESS_H
#define WEBWOWVIEWERCPP_DXTDECOMPRESS_H

#include <cstdint>
enum BC4Mode
{
    BC4_UNORM = 0,
    BC4_SNORM = 1
};

enum BC5Mode
{
    BC5_UNORM = 0,
    BC5_SNORM = 1
};



void DecompressBlockBC1 (uint32_t x, uint32_t y, uint32_t stride,
                         const uint8_t* blockStorage, unsigned char* image);

void DecompressBlockBC2 (uint32_t x, uint32_t y, uint32_t stride,
                         const uint8_t* blockStorage, unsigned char* image);

void DecompressBlockBC3 (uint32_t x, uint32_t y, uint32_t stride,
                         const uint8_t* blockStorage, unsigned char* image);

void DecompressBlockBC5 (uint32_t x, uint32_t y, uint32_t stride, enum BC5Mode mode,
                         const uint8_t* blockStorage, unsigned char* image);

void DecompressBC1(uint32_t width, uint32_t height, const uint8_t* blockStorage, unsigned char* image);

void DecompressBC2(uint32_t width, uint32_t height, const uint8_t* blockStorage, unsigned char* image);
void DecompressBC3(uint32_t width, uint32_t height, const uint8_t* blockStorage, unsigned char* image);

#endif //WEBWOWVIEWERCPP_DXTDECOMPRESS_H
