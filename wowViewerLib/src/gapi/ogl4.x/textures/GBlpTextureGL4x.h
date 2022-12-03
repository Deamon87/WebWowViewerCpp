//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GBLPTEXTURE_4X_H
#define WEBWOWVIEWERCPP_GBLPTEXTURE_4X_H

#include "GTextureGL4x.h"
#include "../GDeviceGL4x.h"

class GBlpTextureGL4x : public GTextureGL4x {
    friend class GDeviceGL4x;
    explicit GBlpTextureGL4x(IDevice &device, HBlpTexture texture, bool xWrapTex, bool yWrapTex);
public:
    ~GBlpTextureGL4x() override;
    void createGlTexture(TextureFormat textureFormat, const HMipmapsVector &hmipmaps) override;

    bool getIsLoaded() override;
    bool postLoad() override {return false;};
private:
    void bind() override; //Should be called only by GDevice
    void unbind();
private:
    HBlpTexture m_texture;

    bool xWrapTex;
    bool yWrapTex;

    bool m_loaded = false;
};


#endif //WEBWOWVIEWERCPP_GBLPTEXTURE_4X_H
