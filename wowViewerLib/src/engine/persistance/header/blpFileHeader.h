//
// Created by deamon on 19.06.17.
//

#ifndef WOWVIEWERLIB_BLPFILEHEADER_H
#define WOWVIEWERLIB_BLPFILEHEADER_H
#include <cstdint>

enum BLPColorEncoding : uint8_t {
    COLOR_JPEG = 0, // not supported
    COLOR_PALETTE = 1,
    COLOR_DXT = 2,
    COLOR_ARGB8888 = 3,
    COLOR_ARGB8888_dup = 4,    // same decompression, likely other PIXEL_FORMAT
};

enum BLPPixelFormat : uint8_t {
    PIXEL_DXT1 = 0,
    PIXEL_DXT3 = 1,
    PIXEL_ARGB8888 = 2,
    PIXEL_ARGB1555 = 3,
    PIXEL_ARGB4444 = 4,
    PIXEL_RGB565 = 5,
    PIXEL_A8 = 6,
    PIXEL_DXT5 = 7,
    PIXEL_UNSPECIFIED = 8,
    PIXEL_ARGB2565 = 9,
    PIXEL_BC5 = 11, // DXGI_FORMAT_BC5_UNORM
    NUM_PIXEL_FORMATS = 12, // (no idea if format=10 exists)
};

struct BlpFile{
    uint32_t fileIdent;
    int32_t version;
    uint8_t colorEncoding;
    uint8_t alphaChannelBitDepth;
    uint8_t preferredFormat;
    uint8_t mipmap_level_and_flags;
    int32_t width;
    int32_t height;
    int32_t offsets[16];
    int32_t lengths[16];
    uint8_t palette[1024];
};
#endif //WOWVIEWERLIB_BLPFILEHEADER_H
