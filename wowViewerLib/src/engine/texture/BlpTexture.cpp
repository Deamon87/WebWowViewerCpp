//
// Created by deamon on 19.06.17.
//

#include <vector>
#include <cmath>
#include <iostream>
#include "../opengl/header.h"

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
        case 3:
            textureFormat = TextureFormat::BGRA;
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
            textureFormat = TextureFormat::BGRA;
            break;
        case 9:
            textureFormat = TextureFormat::PalARGB2565DitherFloydSteinberg;
            break;

        default:
            break;
    }
    return textureFormat;
}
void parseMipmaps(BlpFile *blpFile, TextureFormat textureFormat, MipmapsVector &mipmaps) {
    int32_t width = blpFile->width;
    int32_t height = blpFile->height;

    int minSize = 0;
    if ((textureFormat == TextureFormat::S3TC_RGBA_DXT5) || (textureFormat == TextureFormat::S3TC_RGBA_DXT3)) {
        minSize = (int32_t) (floor((1 + 3) / 4) * floor((1 + 3) / 4) * 16);
    }
    if ((textureFormat == TextureFormat::S3TC_RGB_DXT1) || (textureFormat == TextureFormat::S3TC_RGBA_DXT1)) {
        minSize = (int32_t) (floor((1 + 3) / 4) * floor((1 + 3) / 4) * 8);
    }

    for (int i = 0; i < 15; i++) {
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

        mipmapStruct_t mipmapStruct;
        mipmapStruct.height = height;
        mipmapStruct.width = width;
        mipmapStruct.texture = std::vector<uint8_t>(validSize, 0);

        if ((blpFile->colorEncoding == 1) && (blpFile->preferredFormat == 8)) {//Unk format && pixel format 8
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
        } else {
            std::copy(data, data+blpFile->lengths[i], &mipmapStruct.texture[0]);
        }

        mipmaps.push_back(mipmapStruct);

        height = height / 2;
        width = width / 2;
        height = (height == 0) ? 1 : height;
        width = (width == 0) ? 1 : width;
    }
}

void BlpTexture::process(std::vector<unsigned char> &blpFile, std::string &fileName) {
    /* Post load for texture data. Can't define them through declarative definition */
    /* Determine texture format */
    BlpFile *pBlpFile = (BlpFile *) &blpFile[0];
    if (pBlpFile->fileIdent != '2PLB') {

        std::cout << pBlpFile->fileIdent;
    }
    TextureFormat textureFormat = getTextureType(pBlpFile);

    /* Load texture by mipmaps */
    parseMipmaps(pBlpFile, textureFormat, m_mipmaps);

//    /* Load texture into GL memory */
//    this->texture = createGlTexture(pBlpFile, textureFormat, mipmaps, fileName);
    this->m_isLoaded = true;
}

