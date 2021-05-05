//
// Created by deamon on 19.06.17.
//

#include <vector>
#include <cmath>
#include <iostream>
#include <assert.h>

#include "BlpTexture.h"
#include "DxtDecompress.h"
#include "../persistance/helper/ChunkFileReader.h"

TextureFormat getTextureType(BlpFile *blpFile) {
    TextureFormat textureFormat = TextureFormat::None;
    switch (blpFile->preferredFormat) {
        case 0:
            if (blpFile->alphaChannelBitDepth > 0) {
                textureFormat = TextureFormat::S3TC_RGBA_DXT1;
            } else {
                textureFormat = TextureFormat::S3TC_RGB_DXT1;
            }
            break;
        case 1:
            textureFormat = TextureFormat::S3TC_RGBA_DXT3;
            break;
        case 2:
            textureFormat = TextureFormat::RGBA;
            break;

        case 3:
            textureFormat = TextureFormat::RGBA;
            break;
        case 4:
            textureFormat = TextureFormat::PalARGB1555DitherFloydSteinberg;
            break;
        case 5:
            textureFormat = TextureFormat::PalARGB4444DitherFloydSteinberg;
            break;
        case 7:
            textureFormat = TextureFormat::S3TC_RGBA_DXT5;
            break;
        case 8:
            textureFormat = TextureFormat::RGBA;
            break;
        case 9:
            textureFormat = TextureFormat::PalARGB2565DitherFloydSteinberg;
            break;

        default:
            break;
    }
    return textureFormat;
}
HMipmapsVector parseMipmaps(BlpFile *blpFile, TextureFormat textureFormat) {
    int32_t width = blpFile->width;
    int32_t height = blpFile->height;

    int minSize = 0;
    if ((textureFormat == TextureFormat::S3TC_RGBA_DXT5) || (textureFormat == TextureFormat::S3TC_RGBA_DXT3)) {
        minSize = (int32_t) (floor((1 + 3) / 4) * floor((1 + 3) / 4) * 16);
    }
    if ((textureFormat == TextureFormat::S3TC_RGB_DXT1) || (textureFormat == TextureFormat::S3TC_RGBA_DXT1)) {
        minSize = (int32_t) (floor((1 + 3) / 4) * floor((1 + 3) / 4) * 8);
    }

    int mipmapsCnt = 0;
    for (int i = 0; i < 15; i++) {
        if ((blpFile->lengths[i] == 0) || (blpFile->offsets[i] == 0)) break;
        mipmapsCnt++;
    }
    auto mipmaps = std::make_shared<std::vector<mipmapStruct_t>>();

    mipmaps->resize(mipmapsCnt);

    for (int i = 0; i < mipmapsCnt; i++) {
        if ((blpFile->lengths[i] == 0) || (blpFile->offsets[i] == 0)) break;

        uint8_t *data = ((uint8_t *) blpFile)+blpFile->offsets[i]; //blpFile->lengths[i]);

        //Check dimensions for dxt textures
        int32_t validSize = blpFile->lengths[i];
        if ((textureFormat == TextureFormat::S3TC_RGBA_DXT5) || (textureFormat == TextureFormat::S3TC_RGBA_DXT3)) {
            validSize = (int32_t) (floor((width + 3) / 4) * floor((height + 3) / 4) * 16);
        }
        if ((textureFormat == TextureFormat::S3TC_RGB_DXT1) || (textureFormat == TextureFormat::S3TC_RGBA_DXT1)) {
            validSize = (int32_t) (floor((width + 3) / 4) * floor((height + 3) / 4) * 8);
        }

//        if (minSize == validSize) break;

        mipmapStruct_t &mipmapStruct = (*mipmaps)[i];
        mipmapStruct.height = height;
        mipmapStruct.width = width;
        mipmapStruct.texture.resize(validSize, 0);

        //If the bytes are not compressed
        if (blpFile->preferredFormat == BLPPixelFormat::PIXEL_UNSPECIFIED) {
            //If the
            if (blpFile->colorEncoding == BLPColorEncoding::COLOR_PALETTE) {
                uint8_t *paleteData = data;
                validSize = 4 * width * height;
                mipmapStruct.texture = std::vector<uint8_t>(validSize, 0);

                for (int j = 0; j< width*height; j++) {
                    uint8_t colIndex = paleteData[j];
                    uint8_t b = blpFile->palette[colIndex*4 + 0];
                    uint8_t g = blpFile->palette[colIndex*4 + 1];
                    uint8_t r = blpFile->palette[colIndex*4 + 2];
                    uint8_t a = paleteData[width*height + j];

                    mipmapStruct.texture[j*4 + 0] = r;
                    mipmapStruct.texture[j*4 + 1] = g;
                    mipmapStruct.texture[j*4 + 2] = b;
                    mipmapStruct.texture[j*4 + 3] = a;
                }
            } else if (blpFile->colorEncoding == BLPColorEncoding::COLOR_ARGB8888) {
                //Turn BGRA into RGBA

                validSize = 4 * width * height;
                mipmapStruct.texture = std::vector<uint8_t>(validSize, 0);
                for (int j = 0; j< width*height; j++) {
                    uint8_t b = data[j*4 + 0];
                    uint8_t g = data[j*4 + 1];
                    uint8_t r = data[j*4 + 2];
                    uint8_t a = data[j*4 + 3];

                    mipmapStruct.texture[j*4 + 0] = r;
                    mipmapStruct.texture[j*4 + 1] = g;
                    mipmapStruct.texture[j*4 + 2] = b;
                    mipmapStruct.texture[j*4 + 3] = a;
                }

            } else {
                std::copy(data, data + blpFile->lengths[i], &mipmapStruct.texture[0]);
            }
        }

        height = height / 2;
        width = width / 2;
        height = (height == 0) ? 1 : height;
        width = (width == 0) ? 1 : width;
    }
    return mipmaps;
}

void BlpTexture::process(HFileContent blpFile, const std::string &fileName) {
    /* Post load for texture data. Can't define them through declarative definition */
    /* Determine texture format */
    std::cout << fileName << std::endl;
    BlpFile *pBlpFile = (BlpFile *) &(*blpFile.get())[0];
    if (pBlpFile->fileIdent != '2PLB') {
        std::cout << pBlpFile->fileIdent;
    }
    this->m_textureFormat = getTextureType(pBlpFile);

    /* Load texture by mipmaps */
    assert(this->m_textureFormat != TextureFormat::None);
    m_mipmaps = parseMipmaps(pBlpFile, m_textureFormat);

//    /* Load texture into GL memory */
//    this->texture = createGlTexture(pBlpFile, textureFormat, mipmaps, fileName);
    this->fsStatus = FileStatus ::FSLoaded;
    this->m_textureName = fileName;
}

