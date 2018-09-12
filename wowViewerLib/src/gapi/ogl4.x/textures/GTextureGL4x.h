//
// Created by Deamon on 7/23/2018.
//

#ifndef AWEBWOWVIEWERCPP_GTEXTURE_4X_H
#define AWEBWOWVIEWERCPP_GTEXTURE_4X_H

#include "../GDeviceGL4x.h"
#include "../../interface/textures/ITexture.h"

class GTextureGL4x : public ITexture {
    friend class GDeviceGL4x;
protected:
    explicit GTextureGL4x(IDevice &device);
public:
    ~GTextureGL4x() override;

    void loadData(int width, int height, void *data) override;
    bool getIsLoaded() override;
    void createGlTexture(TextureFormat textureFormat, const MipmapsVector &mipmaps) override {
        throw "Not Implemented in this class";
    }
private:
    void createBuffer();
    void destroyBuffer();
    virtual void bind(); //Should be called only by GDevice
    void unbind();
protected:
    void * pIdentifierBuffer;

    IDevice &m_device;

    bool m_loaded = false;
};


#endif //AWEBWOWVIEWERCPP_GTEXTURE_4X_H
