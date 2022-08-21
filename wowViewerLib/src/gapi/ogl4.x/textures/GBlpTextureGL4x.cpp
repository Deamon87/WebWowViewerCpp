//
// Created by deamon on 05.06.18.
//

#include "../GDeviceGL4x.h"
#include "GBlpTextureGL4x.h"
#include "../../../engine/opengl/header.h"
#include "../../../engine/persistance/helper/ChunkFileReader.h"
#include "../../../engine/texture/DxtDecompress.h"

GBlpTextureGL4x::GBlpTextureGL4x(IDevice &device, HBlpTexture texture, bool xWrapTex, bool yWrapTex)
    : GTextureGL4x(device), m_texture(texture) {
    this->xWrapTex = xWrapTex;
    this->yWrapTex = yWrapTex;
}

GBlpTextureGL4x::~GBlpTextureGL4x() {

}

void GBlpTextureGL4x::bind() {
    glBindTexture(GL_TEXTURE_2D, *(GLuint *) pIdentifierBuffer);
}

void GBlpTextureGL4x::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GBlpTextureGL4x::createGlTexture(TextureFormat textureFormat, const HMipmapsVector &hmipmaps) {
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
            textureGPUFormat = GL_RGBA8;
            break;

        default:
            debuglog("Unknown texture format found in file: ")
            break;
    }
//    }

    /* S3TC is not supported on mobile platforms */
    bool useDXT1Decoding = false;
    bool useDXT3Decoding = false;
    bool useDXT5Decoding = false;

//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    auto &mipmaps = *hmipmaps;

    glTexStorage2D(GL_TEXTURE_2D, mipmaps.size(), textureGPUFormat, mipmaps[0].width, mipmaps[0].height);

    bool generateMipMaps = false;
    switch (textureFormat) {
        case TextureFormat::S3TC_RGB_DXT1:
        case TextureFormat::S3TC_RGBA_DXT1: {
            unsigned char *decodedResult = nullptr;
            if (useDXT1Decoding)
                decodedResult = new unsigned char[mipmaps[0].width * mipmaps[0].height * 4];

            for (int k = 0; k < mipmaps.size(); k++) {
                if (useDXT1Decoding) {
                    DecompressBC1( mipmaps[k].width, mipmaps[k].height, &mipmaps[k].texture[0], decodedResult);
                    glTexImage2D(GL_TEXTURE_2D, k, GL_RGBA, mipmaps[k].width, mipmaps[k].height, 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, decodedResult);
                } else {
                    glCompressedTexSubImage2D(GL_TEXTURE_2D, k, 0, 0,
                                              mipmaps[k].width,
                                              mipmaps[k].height,
                                              textureGPUFormat,
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
                    glCompressedTexSubImage2D(GL_TEXTURE_2D, k, 0, 0,
                                           mipmaps[k].width,
                                           mipmaps[k].height,
                                           textureGPUFormat,
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
                    glCompressedTexSubImage2D(GL_TEXTURE_2D, k, 0, 0,
                                              mipmaps[k].width,
                                              mipmaps[k].height,
                                              textureGPUFormat,
                                           (GLsizei) mipmaps[k].texture.size(),
                                           &mipmaps[k].texture[0]);
                }
            }
            if (useDXT5Decoding)
                delete decodedResult;
            break;
        }

        case TextureFormat::BGRA:
            for( int k = 0; k < mipmaps.size(); k++) {
                glTexSubImage2D(GL_TEXTURE_2D, k, 0,0, mipmaps[k].width, mipmaps[k].height, GL_RGBA, GL_UNSIGNED_BYTE,
                             &mipmaps[k].texture[0]);
            }
            break;
    }
#ifndef WITH_GLESv2
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, (GLint) mipmaps.size()-1);
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
    if (anisFilterExt) {
        float aniso = 0.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
    }

    glGenerateMipmap(GL_TEXTURE_2D);
#endif


}

bool GBlpTextureGL4x::getIsLoaded() {
    if (!m_loaded && m_texture != nullptr && m_texture->getIsLoaded()) {
        m_device.bindTexture(this, 0);
        this->createGlTexture(m_texture->getTextureFormat(), m_texture->getMipmapsVector());
        m_device.bindTexture(nullptr, 0);

        m_loaded = true;
    }
    return m_loaded;
}


