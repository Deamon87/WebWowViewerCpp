//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GBLPTEXTURE_H
#define WEBWOWVIEWERCPP_GBLPTEXTURE_H

#include "../../../gapi/interface/textures/IBlpTexture.h"
#include "GTextureGL20.h"
#include "../GDeviceGL20.h"

class GBlpTextureGL20 : public GTextureGL20 {
    friend class GDeviceGL20;
    explicit GBlpTextureGL20(IDevice &device, HBlpTexture texture, bool xWrapTex, bool yWrapTex);
public:
    ~GBlpTextureGL20() override;
    void createGlTexture(TextureFormat textureFormat, const MipmapsVector &mipmaps) override;

    bool getIsLoaded() override;
    bool postLoad() override;
private:
    void bind() override; //Should be called only by GDevice
    void unbind();
private:
    HBlpTexture m_texture;

    bool xWrapTex;
    bool yWrapTex;

    bool m_loaded = false;
};


#endif //WEBWOWVIEWERCPP_GBLPTEXTURE_H
