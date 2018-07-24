//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GBLPTEXTURE_H
#define WEBWOWVIEWERCPP_GBLPTEXTURE_H

#include "../engine/wowCommonClasses.h"
#include "../engine/texture/BlpTexture.h"
#include "GTexture.h"

class GBlpTexture : public GTexture {
    friend class GDevice;
    explicit GBlpTexture(GDevice &device, HBlpTexture texture, bool xWrapTex, bool yWrapTex);
public:
    ~GBlpTexture() override;
    void createGlTexture(TextureFormat textureFormat, const MipmapsVector &mipmaps);

    virtual bool getIsLoaded() override;
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
