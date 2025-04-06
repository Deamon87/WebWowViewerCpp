//
// Created by deamon on 05.06.18.
//
#include <sstream> //for std::stringstream
#include "GBlpTextureVLK.h"
#include "../../../engine/persistance/helper/ChunkFileReader.h"
#include "../../../engine/texture/DxtDecompress.h"

inline static std::string addrToStr(void *addr) {
    std::stringstream ss;
    ss << (void *)addr;
    return ss.str();
}

std::atomic<int> blpTexturesVulkanLoaded = 0;
std::atomic<int> blpTexturesVulkanSizeLoaded = 0;

GBlpTextureVLK::GBlpTextureVLK(IDeviceVulkan &device,
                               const HBlpTexture &texture,
                               const std::function<void(const std::weak_ptr<GTextureVLK>&)> &onUpdateCallback)
    : GTextureVLK(device, onUpdateCallback), m_texture(texture) {

    std::string blpAddress_str = addrToStr(texture.get());
    std::string selfAddr_str = addrToStr(this);

    m_debugName = "Texture FDID " + texture->getTextureName() + " blp ptr: "+ blpAddress_str + " self ptr :" + selfAddr_str ;
}

GBlpTextureVLK::~GBlpTextureVLK() {
//    std::cout << "destroyed blp text!" << std::endl;
    if (m_loaded && m_uploaded && m_texture && m_texture->getStatus() == FileStatus::FSLoaded) {
        blpTexturesVulkanLoaded.fetch_add(-1);
        blpTexturesVulkanSizeLoaded.fetch_add(-m_texture->getFileSize());
    }
}


static int texturesUploaded = 0;
void GBlpTextureVLK::createTexture(TextureFormat textureFormat, const HMipmapsVector &hmipmaps) {
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

        case TextureFormat::RGBA:
            textureFormatGPU = VK_FORMAT_R8G8B8A8_UNORM;
            break;
        case TextureFormat::BC5_UNORM:
            textureFormatGPU = VK_FORMAT_BC5_UNORM_BLOCK;
            break;

        default:
            debuglog("Unknown texture format found in file: ")
            break;
    }
//    }

    auto &mipmaps = *hmipmaps;

    /* S3TC is not supported on mobile platforms */
    bool compressedTextSupported = m_device.getIsBCCompressedTexturesSupported();
    if (!compressedTextSupported && textureFormat != TextureFormat::BGRA) {
        this->decompressAndUpload(textureFormat, hmipmaps);
        return;
    }

    if (mipmaps.size() > 15) {
//        mipmaps = mipmaps.s
    }

    //Create unitedBuffer
    std::vector<uint8_t> unitedBuffer;
    for (int i = 0; i < mipmaps.size(); i++) {
        std::copy(&mipmaps[i].texture[0], &mipmaps[i].texture[0]+mipmaps[i].texture.size(), std::back_inserter(unitedBuffer));
    }

    GTextureVLK::createTexture(hmipmaps, textureFormatGPU, unitedBuffer);
}

TextureStatus GBlpTextureVLK::postLoad() {
    if (m_loaded) return TextureStatus::TSLoaded;
    if (!m_uploaded) {
        if (m_texture == nullptr) return TextureStatus::TSNotLoaded;
        if (m_texture->getStatus() != FileStatus::FSLoaded) return TextureStatus::TSNotLoaded;
        blpTexturesVulkanLoaded.fetch_add(1);
        blpTexturesVulkanSizeLoaded.fetch_add(m_texture->getFileSize());

        this->createTexture(m_texture->getTextureFormat(), m_texture->getMipmapsVector());
//        m_texture = nullptr;
        return TextureStatus::TSHasUpdates;
    } else {
        return GTextureVLK::postLoad();
    }
}

void GBlpTextureVLK::decompressAndUpload(TextureFormat textureFormat, const HMipmapsVector &hmipmaps) {

}