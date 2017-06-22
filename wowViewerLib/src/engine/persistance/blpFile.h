//
// Created by deamon on 19.06.17.
//

#ifndef WOWVIEWERLIB_BLPFILE_H
#define WOWVIEWERLIB_BLPFILE_H
#include <cstdint>

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
#endif //WOWVIEWERLIB_BLPFILE_H
