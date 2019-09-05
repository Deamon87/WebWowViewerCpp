//
// Created by deamon on 05.06.18.
//

#include "GBlpTextureVLK.h"
#include "../../../engine/persistance/helper/ChunkFileReader.h"
#include "../../../engine/texture/DxtDecompress.h"

GBlpTextureVLK::GBlpTextureVLK(IDevice &device, HBlpTexture texture, bool xWrapTex, bool yWrapTex)
    : GTextureVLK(device), m_texture(texture) {
    this->xWrapTex = xWrapTex;
    this->yWrapTex = yWrapTex;
}

GBlpTextureVLK::~GBlpTextureVLK() {
//    std::cout << "error!" << std::endl;
}


static int texturesUploaded = 0;
void GBlpTextureVLK::createGlTexture(TextureFormat textureFormat, const MipmapsVector &mipmaps) {
//    std::cout << "texturesUploaded = " << texturesUploaded++ << " " << this->m_texture->getTextureName() <<std::endl;

    VkFormat textureFormatGPU;
//     if (ext) {
    switch (textureFormat) {
        case TextureFormat::S3TC_RGB_DXT1:
            textureFormatGPU = VK_FORMAT_BC1_RGB_UNORM_BLOCK;
            break;

        case TextureFormat::S3TC_RGBA_DXT1:
            textureFormatGPU = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
            break;


        case TextureFormat::S3TC_RGBA_DXT3:
            textureFormatGPU = VK_FORMAT_BC2_UNORM_BLOCK;
            break;

        case TextureFormat::S3TC_RGBA_DXT5:
            textureFormatGPU = VK_FORMAT_BC3_UNORM_BLOCK;
            break;

        case TextureFormat::BGRA:
            textureFormatGPU = VK_FORMAT_B8G8R8A8_SNORM;
            break;

        default:
            debuglog("Unknown texture format found in file: ")
            break;
    }
//    }

    /* S3TC is not supported on mobile platforms */
    bool compressedTextSupported = m_device.getIsCompressedTexturesSupported();
    if (!compressedTextSupported && textureFormat !=  TextureFormat::BGRA) {
        this->decompressAndUpload(textureFormat, mipmaps);
        return;
    }

    for (int k = 0; k < mipmaps.size(); k++) {

            DecompressBC2(mipmaps[k].width, mipmaps[k].height, &mipmaps[k].texture[0], decodedResult);
            glTexImage2D(GL_TEXTURE_2D, k, GL_RGBA, mipmaps[k].width, mipmaps[k].height, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, decodedResult);
    }


    bool generateMipMaps = false;
    switch (textureFormat) {
        case TextureFormat::S3TC_RGB_DXT1:
        case TextureFormat::S3TC_RGBA_DXT1: {
            unsigned char *decodedResult = nullptr;
            if (useDXT1Decoding)
                decodedResult = new unsigned char[mipmaps[0].width * mipmaps[0].height * 4];


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

    if (m_device.getIsAnisFiltrationSupported()) {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_device.getAnisLevel());
    }
}



bool GBlpTextureVLK::getIsLoaded() {
    return m_loaded;
}

bool GBlpTextureVLK::postLoad() {
    if (m_loaded) return false;
    if (m_texture == nullptr) return false;
    if (!m_texture->getIsLoaded()) return false;

    m_device.bindTexture(this, 0);
    this->createGlTexture(m_texture->getTextureFormat(), m_texture->getMipmapsVector());
    m_device.bindTexture(nullptr, 0);

    m_loaded = true;
    return true;
}

void createTextureImage() {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);

    stbi_image_free(pixels);

    device->createImage(texWidth, texHeight, mipLevels, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
    copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, mipLevels);
}



void GBlpTextureVLK::decompressAndUpload(TextureFormat textureFormat, const MipmapsVector &mipmaps) {

}