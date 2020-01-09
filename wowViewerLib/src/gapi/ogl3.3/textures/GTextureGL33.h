//
// Created by Deamon on 7/23/2018.
//

#ifndef AWEBWOWVIEWERCPP_GTEXTURE_H
#define AWEBWOWVIEWERCPP_GTEXTURE_H

#include "../GDeviceGL33.h"
#include "../../interface/textures/ITexture.h"
#include "../../../engine/opengl/header.h"

class GTextureGL33 : public ITexture {
    friend class GDeviceGL33;
protected:
    explicit GTextureGL33(IDevice &device);
public:
    ~GTextureGL33() override;

    void loadData(int width, int height, void *data) override;
    bool getIsLoaded() override;
    void createGlTexture(TextureFormat textureFormat, const MipmapsVector &mipmaps) override {
        throw "Not Implemented in this class";
    }
    bool postLoad() override { return false;};
    void* getIdent() override { return reinterpret_cast<void *>(textureIdentifier); };
private:
    void createBuffer();
    void destroyBuffer();
    virtual void bind(); //Should be called only by GDevice
    void unbind();
protected:
    GLuint textureIdentifier;

    GDeviceGL33 &m_device;

    bool m_loaded = false;
};


#endif //AWEBWOWVIEWERCPP_GTEXTURE_H
