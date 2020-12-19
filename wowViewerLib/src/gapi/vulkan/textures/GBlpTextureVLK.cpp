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
            textureFormatGPU = VK_FORMAT_B8G8R8A8_UNORM;
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

    if (mipmaps.size() > 15) {
//        mipmaps = mipmaps.s
    }

    //Create unitedBuffer
    std::vector<uint8_t> unitedBuffer;
    for (int i = 0; i < mipmaps.size(); i++) {
        std::copy(&mipmaps[i].texture[0], &mipmaps[i].texture[0]+mipmaps[i].texture.size(), std::back_inserter(unitedBuffer));
    }

    createTexture(mipmaps, textureFormatGPU, unitedBuffer);
}

//bool GBlpTextureVLK::getIsLoaded() {
//    return m_loaded;
//}

bool GBlpTextureVLK::postLoad() {
    if (m_loaded) return false;
    if (!m_uploaded) {
        if (m_texture == nullptr) return false;
        if (m_texture->getStatus() != FileStatus::FSLoaded) return false;
    }

    if (m_uploaded) {
        return GTextureVLK::postLoad();
    } else {
        this->createGlTexture(m_texture->getTextureFormat(), m_texture->getMipmapsVector());
        m_texture = nullptr;
        return false;
    }
}


void GBlpTextureVLK::decompressAndUpload(TextureFormat textureFormat, const MipmapsVector &mipmaps) {

}