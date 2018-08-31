//
// Created by Deamon on 8/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_IBLPTEXTURE_H
#define AWEBWOWVIEWERCPP_IBLPTEXTURE_H

#include "ITexture.h"

#include "../../../engine/wowCommonClasses.h"
#include "../../../engine/texture/BlpTexture.h"

class IBlpTexture : public virtual ITexture {
    virtual void createGlTexture(TextureFormat textureFormat, const MipmapsVector &mipmaps) = 0;
};
#endif //AWEBWOWVIEWERCPP_IBLPTEXTURE_H
