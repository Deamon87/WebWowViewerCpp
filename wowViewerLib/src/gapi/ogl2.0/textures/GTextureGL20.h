//
// Created by Deamon on 7/23/2018.
//

#ifndef AWEBWOWVIEWERCPP_GTEXTURE_H
#define AWEBWOWVIEWERCPP_GTEXTURE_H

#include "../GDeviceGL20.h"
#include "../../interface/textures/ITexture.h"
#include "../../../engine/opengl/header.h"

class GTextureGL20 : public ITexture {
    friend class GDeviceGL20;
protected:
    explicit GTextureGL20(IDevice &device);
public:
    ~GTextureGL20() override;

    void loadData(int width, int height, void *data) override;
    bool getIsLoaded() override;
    void createGlTexture(TextureFormat textureFormat, const MipmapsVector &mipmaps) override {
        throw "Not Implemented in this class";
    }
    bool postLoad() override { return false;};
private:
    void createBuffer();
    void destroyBuffer();
    virtual void bind(); //Should be called only by GDevice
    void unbind();
protected:
    GLuint textureIdentifier;

    IDevice &m_device;

    bool m_loaded = false;
};


#endif //AWEBWOWVIEWERCPP_GTEXTURE_H
