//
// Created by Deamon on 8/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_ITEXTURE_H
#define AWEBWOWVIEWERCPP_ITEXTURE_H
#include "../../../engine/wowCommonClasses.h"
#include "../../../engine/texture/BlpTexture.h"

class ITexture {
public:
    virtual ~ITexture() = 0;

    virtual void loadData(int width, int height, void *data) = 0;
    virtual bool getIsLoaded() = 0;

    virtual void createGlTexture(TextureFormat textureFormat, const MipmapsVector &mipmaps) = 0;
};
#endif //AWEBWOWVIEWERCPP_ITEXTURE_H
