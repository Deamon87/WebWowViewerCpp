//
// Created by deamon on 05.06.18.
//

#include "../GDeviceGL33.h"
#include "GBlpTextureGL33.h"
#include "../../../engine/opengl/header.h"
#include "../../../engine/persistance/helper/ChunkFileReader.h"
#include "../../../engine/texture/DxtDecompress.h"



GBlpTextureGL33::GBlpTextureGL33(IDevice &device, HBlpTexture texture, bool xWrapTex, bool yWrapTex)
    : GTextureGL33(device, xWrapTex, yWrapTex), m_texture(texture) {

}

GBlpTextureGL33::~GBlpTextureGL33() {
//    std::cout << "error!" << std::endl;
}

void GBlpTextureGL33::bind() {
#if OPENGL_DGB_MESSAGE
    std::string debugMess = "Binding Texture "+m_texture->getTextureName();
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, (GLsizei)((uint64_t)this&0xfffffff), GLsizei(debugMess.size()), debugMess.c_str());

        glDebugMessageInsert( GL_DEBUG_SOURCE_APPLICATION,
                              GL_DEBUG_TYPE_MARKER, 1,
                              GL_DEBUG_SEVERITY_LOW,
                              GLsizei(debugMess.size()),
                              debugMess.c_str()
        );
#endif

    glBindTexture(GL_TEXTURE_2D, textureIdentifier);
#if OPENGL_DGB_MESSAGE
    glPopDebugGroup();
#endif
}

void GBlpTextureGL33::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

static int texturesUploaded = 0;
void GBlpTextureGL33::createGlTexture(TextureFormat textureFormat, const HMipmapsVector &hmipmaps) {
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

//    std::cout
//        << "useDXT1Decoding = " << useDXT1Decoding << " "
//        << "useDXT3Decoding = " << useDXT3Decoding << " "
//        << "useDXT5Decoding = " << useDXT5Decoding << " " << std::endl;

//    useDXT1Decoding = true; // Note: manual DXT1 decompression loses alpha channel for S3TC_RGBA_DXT1 textures
//    useDXT3Decoding = true;
//    useDXT5Decoding = true;

//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);

    bool skipGenerationOfMipMaps = true;
    auto &mipmaps = *hmipmaps;

    logGLError
    bool generateMipMaps = false;
    switch (textureFormat) {
        case TextureFormat::S3TC_RGB_DXT1:
        case TextureFormat::S3TC_RGBA_DXT1: {
            std::vector<unsigned char> decodedResult;
            if (useDXT1Decoding)
                decodedResult = std::vector<unsigned char>(mipmaps[0].width * mipmaps[0].height * 4);

            for (int k = 0; k < mipmaps.size(); k++) {
                logGLError
                if (useDXT1Decoding) {
                    bool hasAlpha = (textureFormat == TextureFormat::S3TC_RGBA_DXT1);
                    DecompressBC1( mipmaps[k].width, mipmaps[k].height, &mipmaps[k].texture[0], decodedResult.data(), hasAlpha);
                    glTexImage2D(GL_TEXTURE_2D, k, GL_RGBA, mipmaps[k].width, mipmaps[k].height, 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, decodedResult.data());
                } else {
                    glCompressedTexImage2D(GL_TEXTURE_2D, k, textureGPUFormat, mipmaps[k].width,
                                           mipmaps[k].height, 0,
                                           (GLsizei) mipmaps[k].texture.size(),
                                           &mipmaps[k].texture[0]);
                }
                logGLError
            }

            break;
        }


        case TextureFormat::S3TC_RGBA_DXT3: {
            std::vector<unsigned char> decodedResult;
            if (useDXT3Decoding)
                decodedResult = std::vector<unsigned char>(mipmaps[0].width * mipmaps[0].height * 4);

            logGLError
            for (int k = 0; k < mipmaps.size(); k++) {
                if (useDXT3Decoding) {
                    DecompressBC2(mipmaps[k].width, mipmaps[k].height, &mipmaps[k].texture[0], decodedResult.data());
                    glTexImage2D(GL_TEXTURE_2D, k, GL_RGBA, mipmaps[k].width, mipmaps[k].height, 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, decodedResult.data());
                } else {
                    glCompressedTexImage2D(GL_TEXTURE_2D, k, textureGPUFormat, mipmaps[k].width,
                                           mipmaps[k].height, 0,
                                           (GLsizei) mipmaps[k].texture.size(),
                                           &mipmaps[k].texture[0]);
                }
                logGLError
            }

            break;
        }

        case TextureFormat::S3TC_RGBA_DXT5: {
            std::vector<unsigned char> decodedResult;
            if (useDXT5Decoding)
                decodedResult = std::vector<unsigned char>(mipmaps[0].width * mipmaps[0].height * 4);

            logGLError
            for (int k = 0; k < mipmaps.size(); k++) {
                if (useDXT5Decoding) {
                    DecompressBC3(mipmaps[k].width, mipmaps[k].height, &mipmaps[k].texture[0], decodedResult.data());
                    glTexImage2D(GL_TEXTURE_2D, k, GL_RGBA, mipmaps[k].width, mipmaps[k].height, 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, decodedResult.data());
                } else {
                    glCompressedTexImage2D(GL_TEXTURE_2D, k, textureGPUFormat, mipmaps[k].width,
                                           mipmaps[k].height, 0,
                                           (GLsizei) mipmaps[k].texture.size(),
                                           &mipmaps[k].texture[0]);
                }
                logGLError
            }
            break;
        }

        case TextureFormat::RGBA:
        logGLError
            for( int k = 0; k < mipmaps.size(); k++) {
                glTexImage2D(GL_TEXTURE_2D, k, GL_RGBA, mipmaps[k].width, mipmaps[k].height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                             &mipmaps[k].texture[0]);
                logGLError
            }
            skipGenerationOfMipMaps = false;
            break;
        case TextureFormat::None:
        case TextureFormat::PalARGB1555DitherFloydSteinberg:
        case TextureFormat::PalARGB4444DitherFloydSteinberg:
        case TextureFormat::PalARGB2565DitherFloydSteinberg:
            std::cout << "Detected unhandled texture format" << std::endl;
        break;
    }
#ifndef WITH_GLESv2
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, (GLint) mipmaps.size()-1);
#endif
    logGLError
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    logGLError
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    logGLError
    if (xWrapTex) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    }
    logGLError
    if (yWrapTex) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    logGLError
    bool anisFilterExt = true;
#ifndef WITH_GLESv2
    if (m_device.getIsAnisFiltrationSupported()) {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_device.getAnisLevel());
    }
#endif
    logGLError
    if (!skipGenerationOfMipMaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    logGLError
}

bool GBlpTextureGL33::getIsLoaded() {
    return m_loaded;
}

bool GBlpTextureGL33::postLoad() {
    if (m_loaded) return false;
    if (m_texture == nullptr) return false;
    if (m_texture->getStatus() != FileStatus::FSLoaded) return false;

    m_device.bindTexture(this, 0);
    this->createGlTexture(m_texture->getTextureFormat(), m_texture->getMipmapsVector());
    m_device.bindTexture(nullptr, 0);

//    m_texture = nullptr;

    m_loaded = true;
    return true;
}


