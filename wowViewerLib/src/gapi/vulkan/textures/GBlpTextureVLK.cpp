//
// Created by deamon on 05.06.18.
//

#include "GBlpTextureVLK.h"
#include "../../../engine/persistance/helper/ChunkFileReader.h"
#include "../../../engine/texture/DxtDecompress.h"

GBlpTextureVLK::GBlpTextureVLK(IDevice &device, HBlpTexture texture, bool xWrapTex, bool yWrapTex)
    : GTextureVLK(device,xWrapTex,yWrapTex), m_texture(texture) {
}

GBlpTextureVLK::~GBlpTextureVLK() {
//    std::cout << "error!" << std::endl;
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

        default:
            debuglog("Unknown texture format found in file: ")
            break;
    }
//    }

    auto &mipmaps = *hmipmaps;

    /* S3TC is not supported on mobile platforms */
    bool compressedTextSupported = m_device.getIsCompressedTexturesSupported();
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

bool GBlpTextureVLK::postLoad() {
    if (m_loaded) return false;
    if (!m_uploaded) {
        if (m_texture == nullptr) return false;
        if (m_texture->getStatus() != FileStatus::FSLoaded) return false;
    }

    if (m_uploaded) {
        return GTextureVLK::postLoad();
    } else {
        this->createTexture(m_texture->getTextureFormat(), m_texture->getMipmapsVector());
//        m_texture = nullptr;
        return false;
    }
}


void GBlpTextureVLK::decompressAndUpload(TextureFormat textureFormat, const HMipmapsVector &hmipmaps) {

}