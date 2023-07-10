//
// Created by Deamon on 8/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_ITEXTURE_H
#define AWEBWOWVIEWERCPP_ITEXTURE_H

#include "../../../engine/wowCommonClasses.h"
#include "../../../engine/texture/BlpTexture.h"

enum class ITextureFormat {
    itNone,
    itRGBA,
    itRGBAFloat32,
    itDepth32
};

enum class TextureStatus {
    TSNotLoaded,
    TSHasUpdates,
    TSLoaded
};

class ITexture {
public:
    virtual ~ITexture() {};
    ITexture() = default;
    ITexture(const ITexture&) = delete;
    ITexture(const ITexture&&) = delete;

    virtual void loadData(int width, int height, void *data, ITextureFormat textureFormat) = 0;
    virtual void readData(std::vector<uint8_t> &buff) = 0;

    virtual bool getIsLoaded() = 0;
    virtual TextureStatus postLoad() = 0;

    virtual void createTexture(TextureFormat textureFormat, const HMipmapsVector &mipmaps) = 0;
};

extern std::atomic<int> blpTexturesVulkanLoaded;
extern std::atomic<int> blpTexturesVulkanSizeLoaded;
#endif //AWEBWOWVIEWERCPP_ITEXTURE_H
