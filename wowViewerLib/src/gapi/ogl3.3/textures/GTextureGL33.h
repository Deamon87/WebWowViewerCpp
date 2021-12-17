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
    explicit GTextureGL33(const HGDevice &device, bool xWrapTex, bool yWrapTex);
public:
    ~GTextureGL33() override;

    void loadData(int width, int height, void *data, ITextureFormat textureFormat) override;
    void readData(std::vector<uint8_t> &buff) override;
    bool getIsLoaded() override;
    void createGlTexture(TextureFormat textureFormat, const HMipmapsVector &mipmaps) override {
//        throw "Not Implemented in this class";
    }
    bool postLoad() override { return false;};
    void bindToCurrentFrameBufferAsColor(uint8_t attachmentIndex);
    void bindToCurrentFrameBufferAsDepth();
private:
    void createBuffer();
    void destroyBuffer();
    virtual void bind(); //Should be called only by GDevice
    void unbind();
protected:
    GLuint textureIdentifier = 0;

    HGDevice m_device;

    bool m_loaded = false;
    int width = 0;
    int height = 0;

    bool xWrapTex;
    bool yWrapTex;
};


#endif //AWEBWOWVIEWERCPP_GTEXTURE_H
