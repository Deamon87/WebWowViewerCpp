//
// Created by deamon on 05.06.18.
//

#include "../GDeviceGL20.h"
#include "GBlpTextureGL20.h"
#include "../../../engine/opengl/header.h"
#include "../../../engine/persistance/helper/ChunkFileReader.h"
#include "../../../engine/texture/DxtDecompress.h"

GBlpTextureGL20::GBlpTextureGL20(IDevice &device, HBlpTexture texture, bool xWrapTex, bool yWrapTex)
    : GTextureGL20(device, xWrapTex, yWrapTex), m_texture(texture) {
}

GBlpTextureGL20::~GBlpTextureGL20() {
//    std::cout << "error!" << std::endl;
}

void GBlpTextureGL20::bind() {
    glBindTexture(GL_TEXTURE_2D, textureIdentifier);
}

void GBlpTextureGL20::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

static int texturesUploaded = 0;
void GBlpTextureGL20::createTexture(TextureFormat textureFormat, const HMipmapsVector &hmipmaps) {
//    std::cout << "texturesUploaded = " << texturesUploaded++ << " " << this->m_texture->getTextureName() <<std::endl;

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

        case TextureFormat::BGRA:
            break;

        default:
            debuglog("Unknown texture format found in file: ")
            break;
    }
//    }

    /* S3TC is not supported on mobile platforms */
    bool useDXT1Decoding = !m_device.getIsCompressedTexturesSupported();
    bool useDXT3Decoding = !m_device.getIsCompressedTexturesSupported();
    bool useDXT5Decoding = !m_device.getIsCompressedTexturesSupported();

//    useDXT1Decoding = true;
//    useDXT3Decoding = true;
//    useDXT5Decoding = true;

//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);

    auto &mipmaps = *hmipmaps;
    bool generateMipMaps = false;
    switch (textureFormat) {
        case TextureFormat::S3TC_RGB_DXT1:
        case TextureFormat::S3TC_RGBA_DXT1: {
            unsigned char *decodedResult = nullptr;
            if (useDXT1Decoding)
                decodedResult = new unsigned char[mipmaps[0].width * mipmaps[0].height * 4];

            for (int k = 0; k < mipmaps.size(); k++) {
                if (useDXT1Decoding) {
                    bool hasAlpha = (textureFormat == TextureFormat::S3TC_RGBA_DXT1);
                    DecompressBC1( mipmaps[k].width, mipmaps[k].height, &mipmaps[k].texture[0], decodedResult, hasAlpha);
                    glTexImage2D(GL_TEXTURE_2D, k, GL_RGBA, mipmaps[k].width, mipmaps[k].height, 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, decodedResult);
                } else {
                    glCompressedTexImage2D(GL_TEXTURE_2D, k, textureGPUFormat, mipmaps[k].width,
                                           mipmaps[k].height, 0,
                                           (GLsizei) mipmaps[k].texture.size(),
                                           &mipmaps[k].texture[0]);
                }
            }
            if (useDXT1Decoding)
                delete decodedResult;

            break;
        }


        case TextureFormat::S3TC_RGBA_DXT3: {
            unsigned char *decodedResult = nullptr;
            if (useDXT3Decoding)
                decodedResult = new unsigned char[mipmaps[0].width * mipmaps[0].height * 4];

            for (int k = 0; k < mipmaps.size(); k++) {
                if (useDXT3Decoding) {
                    DecompressBC2(mipmaps[k].width, mipmaps[k].height, &mipmaps[k].texture[0], decodedResult);
                    glTexImage2D(GL_TEXTURE_2D, k, GL_RGBA, mipmaps[k].width, mipmaps[k].height, 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, decodedResult);
                } else {
                    glCompressedTexImage2D(GL_TEXTURE_2D, k, textureGPUFormat, mipmaps[k].width,
                                           mipmaps[k].height, 0,
                                           (GLsizei) mipmaps[k].texture.size(),
                                           &mipmaps[k].texture[0]);
                }
            }
            if (useDXT3Decoding)
                delete decodedResult;

            break;
        }

        case TextureFormat::S3TC_RGBA_DXT5: {
            unsigned char *decodedResult = nullptr;
            if (useDXT5Decoding)
                decodedResult = new unsigned char[mipmaps[0].width * mipmaps[0].height * 4];
            for (int k = 0; k < mipmaps.size(); k++) {
                if (useDXT5Decoding) {
                    DecompressBC3(mipmaps[k].width, mipmaps[k].height, &mipmaps[k].texture[0], decodedResult);
                    glTexImage2D(GL_TEXTURE_2D, k, GL_RGBA, mipmaps[k].width, mipmaps[k].height, 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, decodedResult);
                } else {
                    glCompressedTexImage2D(GL_TEXTURE_2D, k, textureGPUFormat, mipmaps[k].width,
                                           mipmaps[k].height, 0,
                                           (GLsizei) mipmaps[k].texture.size(),
                                           &mipmaps[k].texture[0]);
                }
            }
            if (useDXT5Decoding)
                delete decodedResult;
            break;
        }

        case TextureFormat::RGBA:
            for( int k = 0; k < mipmaps.size(); k++) {
                glTexImage2D(GL_TEXTURE_2D, k, GL_RGBA, mipmaps[k].width, mipmaps[k].height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                             &mipmaps[k].texture[0]);
            }
            break;
        case TextureFormat::None:
        case TextureFormat::BGRA:
        case TextureFormat::PalARGB1555DitherFloydSteinberg:
        case TextureFormat::PalARGB4444DitherFloydSteinberg:
        case TextureFormat::PalARGB2565DitherFloydSteinberg:
            std::cout << "Detected unhandled texture format" << std::endl;
        break;
    }
#ifndef WITH_GLESv2
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, (GLint) mipmaps.size()-1);
#endif
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (xWrapTex) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    }
    if (yWrapTex) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    bool anisFilterExt = true;
#ifndef WITH_GLESv2
    if (m_device.getIsAnisFiltrationSupported()) {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_device.getAnisLevel());
    }
#ifndef __EMSCRIPTEN__
    glGenerateMipmap(GL_TEXTURE_2D);
#endif
#endif


}

bool GBlpTextureGL20::getIsLoaded() {
    return m_loaded;
}

bool GBlpTextureGL20::postLoad() {
    if (m_loaded) return false;
    if (m_texture == nullptr) return false;
    if (m_texture->getStatus() != FileStatus::FSLoaded) return false;

    m_device.bindTexture(this, 0);
    this->createTexture(m_texture->getTextureFormat(), m_texture->getMipmapsVector());
    m_device.bindTexture(nullptr, 0);

    m_loaded = true;
    return true;
}


