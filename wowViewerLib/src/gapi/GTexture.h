//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GTEXTURE_H
#define WEBWOWVIEWERCPP_GTEXTURE_H


#include "../engine/wowCommonClasses.h"
#include "../engine/texture/BlpTexture.h"

class GTexture {
    friend class GDevice;
    GTexture(GDevice &device, HBlpTexture &texture);
public:
    ~GTexture();
    void createGlTexture(TextureFormat textureFormat, MipmapsVector &mipmaps, std::string &filename);
private:


    void createBuffer();
    void destroyBuffer();
    void bind(); //Should be called only by GDevice
    void unbind();

private:
    void * pIdentifierBuffer;

    GDevice &m_device;
    HBlpTexture m_texture;
};


#endif //WEBWOWVIEWERCPP_GTEXTURE_H
