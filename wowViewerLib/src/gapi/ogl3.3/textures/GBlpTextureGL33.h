//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GBLPTEXTURE_H
#define WEBWOWVIEWERCPP_GBLPTEXTURE_H

#include "../../../gapi/interface/textures/IBlpTexture.h"
#include "GTextureGL33.h"
#include "../GDeviceGL33.h"

class GBlpTextureGL33 : public GTextureGL33 {
    friend class GDeviceGL33;
    explicit GBlpTextureGL33(IDevice &device, HBlpTexture texture, bool xWrapTex, bool yWrapTex);
public:
    ~GBlpTextureGL33() override;
    void createGlTexture(TextureFormat textureFormat, const MipmapsVector &mipmaps) override;

    bool getIsLoaded() override;
    bool postLoad() override;
private:
    void bind() override; //Should be called only by GDevice
    void unbind();
private:
    HBlpTexture m_texture;

    bool m_loaded = false;
};


#endif //WEBWOWVIEWERCPP_GBLPTEXTURE_H
