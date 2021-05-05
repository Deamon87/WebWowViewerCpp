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

class ITexture {
public:
    virtual ~ITexture() {};

    virtual void loadData(int width, int height, void *data, ITextureFormat textureFormat) = 0;
    virtual void readData(std::vector<uint8_t> &buff) = 0;

    virtual bool getIsLoaded() = 0;
    virtual bool postLoad() = 0;



    virtual void createGlTexture(TextureFormat textureFormat, const HMipmapsVector &mipmaps) = 0;
};
#endif //AWEBWOWVIEWERCPP_ITEXTURE_H
