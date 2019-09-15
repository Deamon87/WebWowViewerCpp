//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GBLPTEXTUREVLK_H
#define WEBWOWVIEWERCPP_GBLPTEXTUREVLK_H

#include "GTextureVLK.h"
#include "../GDeviceVulkan.h"

class GBlpTextureVLK : public GTextureVLK {
    friend class GDeviceVLK;
    explicit GBlpTextureVLK(IDevice &device, HBlpTexture texture, bool xWrapTex, bool yWrapTex);
public:
    ~GBlpTextureVLK() override;
    void createGlTexture(TextureFormat textureFormat, const MipmapsVector &mipmaps) override;

    bool postLoad() override;
private:
    void decompressAndUpload(TextureFormat textureFormat, const MipmapsVector &mipmaps);
private:
    HBlpTexture m_texture;

    bool xWrapTex;
    bool yWrapTex;
};


#endif //WEBWOWVIEWERCPP_GBLPTEXTUREVLK_H
