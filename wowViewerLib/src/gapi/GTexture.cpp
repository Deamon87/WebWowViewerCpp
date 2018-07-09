//
// Created by deamon on 05.06.18.
//

#include "GTexture.h"
#include "../engine/opengl/header.h"
#include "../engine/persistance/helper/ChunkFileReader.h"
#include "../engine/texture/DxtDecompress.h"

GTexture::GTexture(GDevice &device, HBlpTexture &texture) : m_device(device), m_texture(texture) {
    pIdentifierBuffer = new GLuint;
    createBuffer();
}

GTexture::~GTexture() {
    destroyBuffer();
    delete (GLuint *) pIdentifierBuffer;
}

void GTexture::createBuffer() {
    glGenTextures(1, (GLuint *)pIdentifierBuffer);
}

void GTexture::destroyBuffer() {
    glDeleteTextures(1, (GLuint *)pIdentifierBuffer);
}

void GTexture::bind() {
    glBindTexture(GL_TEXTURE_2D, *(GLuint *)pIdentifierBuffer);
}

void GTexture::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GTexture::createGlTexture(TextureFormat textureFormat, MipmapsVector &mipmaps, std::string &filename) {
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
            debuglog("Unknown texture format found in file: "+filename)
            break;
    }
//    }

    /* S3TC is not supported on mobile platforms */
    bool useDXT1Decoding = false;
    bool useDXT3Decoding = false;
    bool useDXT5Decoding = false;

//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);

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

        case TextureFormat::BGRA:
            for( int k = 0; k < mipmaps.size(); k++) {
                glTexImage2D(GL_TEXTURE_2D, k, GL_RGBA, mipmaps[k].width, mipmaps[k].height, 0, GL_BGRA, GL_UNSIGNED_BYTE,
                             &mipmaps[k].texture[0]);
            }
            break;
    }
#ifndef WITH_GLESv2
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, (GLint) mipmaps.size()-1);
#endif
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    bool anisFilterExt = true;
    if (anisFilterExt) {
        float aniso = 0.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
    }

#ifdef WITH_GLESv2
    glGenerateMipmap(GL_TEXTURE_2D);
#endif


}


