//
// Created by deamon on 19.06.17.
//

#include <vector>
#include <cmath>
#include <GL/gl.h>
#include <GL/glcorearb.h>
#include "BlpTexture.h"
enum class TextureFormat {
    None,
    S3TC_RGBA_DXT1,
    S3TC_RGB_DXT1,
    S3TC_RGBA_DXT3,
    S3TC_RGBA_DXT5,
    BGRA,
    PalARGB1555DitherFloydSteinberg,
    PalARGB4444DitherFloydSteinberg,
    PalARGB2565DitherFloydSteinberg
};

struct mipmapStruct_t {
    std::vector<uint8_t> texture;
    int32_t width;
    int32_t height;
};
typedef std::vector<mipmapStruct_t> MipmapsVector;

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

        mipmapStruct_t mipmapStruct;
        mipmapStruct.height = height;
        mipmapStruct.width = width;
        mipmapStruct.texture = std::vector<uint8_t>(validSize);

        if ((blpFile->colorEncoding == 1) && (blpFile->preferredFormat == 8)) {//Unk format && pixel format 8
            uint8_t *paleteData = data;

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
            mipmapStruct.texture.assign(data, data+blpFile->lengths[i]);
        }

        mipmaps.push_back(mipmapStruct);

        height = floor(height / 2);
        width = floor(width / 2);
        height = (height == 0) ? 1 : height;
        width = (width == 0) ? 1 : width;
    }

}
GLuint createGlTexture(BlpFile *blpFile, TextureFormat textureFormat, MipmapsVector &mipmaps) {
    bool hasAlpha = blpFile->alphaChannelBitDepth > 0;

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    GLuint textureGPUFormat = 0;
//     if (ext) {
        switch (textureFormat) {
            case TextureFormat::S3TC_RGB_DXT1:
                textureGPUFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
                break;

            case TextureFormat::S3TC_RGBA_DXT1:
                textureGPUFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
                break;


            case TextureFormat::S3TC_RGBA_DXT3:
                textureGPUFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
                break;

            case TextureFormat::S3TC_RGBA_DXT5:
                textureGPUFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
                break;
        }
//    }

    /* S3TC is not supported on mobile platforms */
//    var useDXT1Decoding = ((!ext) || (!ext.COMPRESSED_RGB_S3TC_DXT1_EXT)) || ((!ext) || (!ext.COMPRESSED_RGBA_S3TC_DXT1_EXT));
//    var useDXT3Decoding = ((!ext) || (!ext.COMPRESSED_RGBA_S3TC_DXT3_EXT));
//    var useDXT5Decoding = ((!ext) || (!ext.COMPRESSED_RGBA_S3TC_DXT5_EXT));

    bool useDXT1Decoding = false;
    bool useDXT3Decoding = false;
    bool useDXT5Decoding = false;


    /* Hack for DXT1. It still falls on gpu when width is not equal to height and one of them is less than 8 :/ */
    /*
    useDXT1Decoding = useDXT1Decoding ||
        (
            (textureFormat === "S3TC_RGB_DXT1") ||
            (textureFormat === "S3TC_RGBA_DXT1")
        ) &&( mipmaps[0].width !== mipmaps[0].height );
    */

    bool generateMipMaps = false;
    switch (textureFormat) {
        case TextureFormat::S3TC_RGB_DXT1:
        case TextureFormat::S3TC_RGBA_DXT1:
            for( int k = 0; k < mipmaps.size(); k++) {
                if (useDXT1Decoding) {
//                    var decodedResult = decodeDxt(new DataView(mipmaps[k].texture.buffer), mipmaps[k].width, mipmaps[k].height, 'dxt1');
//
//                    gl.texImage2D(gl.TEXTURE_2D, k, gl.RGBA, mipmaps[k].width, mipmaps[k].height, 0, gl.RGBA, gl.UNSIGNED_BYTE, decodedResult);
                } else {
                    glCompressedTexImage2D(GL_TEXTURE_2D, k, textureGPUFormat, mipmaps[k].width, mipmaps[k].height, 0,
                                           mipmaps[k].texture.size(), &mipmaps[k].texture[0]);
                }
            }
            break;

        case TextureFormat::S3TC_RGBA_DXT3:
            for( int k = 0; k < mipmaps.size(); k++) {
                if (useDXT3Decoding) {
//                    var decodedResult = decodeDxt(new DataView(mipmaps[k].texture.buffer), mipmaps[k].width, mipmaps[k].height, 'dxt3');
//                    //var decodedResult = dxtLib.decodeDXT3toBitmap32(mipmaps[k].texture , mipmaps[k].width, mipmaps[k].height);
//                    gl.texImage2D(gl.TEXTURE_2D, k, gl.RGBA, mipmaps[k].width, mipmaps[k].height, 0, gl.RGBA, gl.UNSIGNED_BYTE,  decodedResult);
                } else {
                    glCompressedTexImage2D(GL_TEXTURE_2D, k, textureGPUFormat, mipmaps[k].width, mipmaps[k].height, 0,
                                           mipmaps[k].texture.size(), &mipmaps[k].texture[0]);
                }
            }

            break;

        case TextureFormat::S3TC_RGBA_DXT5:
            for( int k = 0; k < mipmaps.size(); k++) {
                if (useDXT5Decoding) {
//                    var decodedResult = decodeDxt(new DataView(mipmaps[k].texture.buffer), mipmaps[k].width, mipmaps[k].height, 'dxt5');
//                    //var decodedResult = dxtLib.decodeDXT5toBitmap32(mipmaps[k].texture , mipmaps[k].width, mipmaps[k].height);
//                    gl.texImage2D(gl.TEXTURE_2D, k, gl.RGBA, mipmaps[k].width, mipmaps[k].height, 0, gl.RGBA, gl.UNSIGNED_BYTE,  decodedResult);
                } else {
                    glCompressedTexImage2D(GL_TEXTURE_2D, k, textureGPUFormat, mipmaps[k].width, mipmaps[k].height, 0,
                                           mipmaps[k].texture.size(), &mipmaps[k].texture[0]);
                }
            }

            break;

        case TextureFormat::BGRA:
            for( int k = 0; k < mipmaps.size(); k++) {
                glTexImage2D(GL_TEXTURE_2D, k, GL_RGBA, mipmaps[k].width, mipmaps[k].height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                             &mipmaps[k].texture[0]);
            }
            break;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    bool anisFilterExt = true;
    if (anisFilterExt) {
        float aniso = 0.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);

//        var max_anisotropy = this.sceneApi.extensions.getMaxTextAnisotropic();
//        gl.texParameterf(GL_TEXTURE_2D, anisFilterExt.TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropy);
    }

//    if (generateMipMaps) {
//        glGenerateMipmap(GL_TEXTURE_2D);
//    }

    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

void BlpTexture::process(std::vector<unsigned char> &blpFile) {
    /* Post load for texture data. Can't define them through declarative definition */
    /* Determine texture format */
    BlpFile *pBlpFile = (BlpFile *) &blpFile[0];
    TextureFormat textureFormat = getTextureType(pBlpFile);


    /* Load texture by mipmaps */
    MipmapsVector mipmaps;
    parseMipmaps(pBlpFile, textureFormat, mipmaps);

    /* Load texture into GL memory */
    this->texture = createGlTexture(pBlpFile, textureFormat, mipmaps);
    this->m_isLoaded = true;
}

