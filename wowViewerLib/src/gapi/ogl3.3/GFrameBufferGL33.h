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
    ~GFrameBufferGL33() override;


    HGTexture getAttachment(int index) override;
    HGTexture getDepthTexture() override;
    void bindFrameBuffer() override;
    void copyRenderBufferToTexture() override;


private:
    IDevice &mdevice;

    std::vector<HGTexture> attachmentTextures;
    HGTexture depthTexture;


    GLuint m_renderBufFbo;
    ITextureFormat m_depthAttachment;
    std::vector<GLuint> renderBufferAttachments;
    GLuint depthBufferAttachment = 0;

    GLuint m_textureFbo;

    int m_width = 0;
    int m_height = 0;
};


#endif //AWEBWOWVIEWERCPP_GFRAMEBUFFERGL33_H
