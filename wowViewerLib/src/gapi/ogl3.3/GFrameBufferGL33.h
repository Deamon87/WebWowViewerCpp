//
// Created by Deamon on 14.05.2020.
//

#ifndef AWEBWOWVIEWERCPP_GFRAMEBUFFERGL33_H
#define AWEBWOWVIEWERCPP_GFRAMEBUFFERGL33_H


#include <vector>
#include "../interface/IFrameBuffer.h"
#include "../interface/textures/ITexture.h"
#include "GDeviceGL33.h"
#include "../../engine/opengl/header.h"

class GFrameBufferGL33 : public IFrameBuffer {
public:
    GFrameBufferGL33(IDevice &device, std::vector<ITextureFormat> textureAttachments, ITextureFormat depthAttachment, int width, int height);
    ~GFrameBufferGL33();


    HGTexture getAttachment(int index) override;
    HGTexture getDepthTexture() override;
    void bindFrameBuffer();

private:
    IDevice &mdevice;

    std::vector<HGTexture> attachmentTextures;
    HGTexture depthTexture;


    GLuint m_fbo;
};


#endif //AWEBWOWVIEWERCPP_GFRAMEBUFFERGL33_H
