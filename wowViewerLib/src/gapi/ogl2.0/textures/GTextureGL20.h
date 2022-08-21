//
// Created by Deamon on 7/23/2018.
//

#ifndef AWEBWOWVIEWERCPP_GTEXTURE20_H
#define AWEBWOWVIEWERCPP_GTEXTURE20_H

#include "../GDeviceGL20.h"
#include "../../interface/textures/ITexture.h"
#include "../../../engine/opengl/header.h"

class GTextureGL20 : public ITexture {
    friend class GDeviceGL20;
protected:
    explicit GTextureGL20(IDevice &device, bool xWrapTex, bool yWrapTex);
public:
    ~GTextureGL20() override;

    void loadData(int width, int height, void *data, ITextureFormat textureFormat) override;
    void readData(std::vector<uint8_t> &buff) override {};
    bool getIsLoaded() override;
    void createGlTexture(TextureFormat textureFormat, const HMipmapsVector &mipmaps) override {
//        throw "Not Implemented in this class";
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
    bool xWrapTex;
    bool yWrapTex;
};


#endif //AWEBWOWVIEWERCPP_GTEXTURE_H
