//
// Created by Deamon on 14.05.2020.
//

#include "GFrameBufferGL33.h"


GFrameBufferGL33::GFrameBufferGL33 (
    IDevice &device,
    std::vector<ITextureFormat> textureAttachments,
    ITextureFormat depthAttachment,
    int width, int height) : mdevice(device), m_height(height), m_width(width) {

    attachmentTextures = std::vector<HGTexture>(textureAttachments.size());
    for (int i = 0; i < textureAttachments.size(); i++) {
        if (textureAttachments[i] != ITextureFormat::itNone) {
            attachmentTextures[i] = mdevice.createTexture();
            attachmentTextures[i]->loadData(width, height, nullptr, textureAttachments[i]);
        }
    }

    if (depthAttachment != ITextureFormat::itNone) {
        depthTexture = mdevice.createTexture();
        depthTexture->loadData(width, height, nullptr, depthAttachment);
    }

    glGenFramebuffers(1, &m_renderBufFbo);
    glGenFramebuffers(1, &m_textureFbo);

    //1. First fill framebuffer with renderbuffers
    this->bindFrameBuffer();
    renderBufferAttachments = std::vector<GLuint>(textureAttachments.size());
    for (int i = 0; i < renderBufferAttachments.size(); i++) {
        glGenRenderbuffers(1, &renderBufferAttachments[i]);
        glBindRenderbuffer(GL_RENDERBUFFER, renderBufferAttachments[i]);

        if (textureAttachments[i] == ITextureFormat::itRGBA) {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, device.getMaxSamplesCnt(), GL_RGBA8, width, height);
        } else if (textureAttachments[i] == ITextureFormat::itRGBAFloat32)  {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, device.getMaxSamplesCnt(), GL_RGBA32F, width, height);
        }else if (textureAttachments[i] == ITextureFormat::itDepth32)  {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, device.getMaxSamplesCnt(),  GL_DEPTH_COMPONENT32F, width, height);
        }
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_RENDERBUFFER, renderBufferAttachments[i]);
    }
    if (depthAttachment != ITextureFormat::itNone) {
        glGenRenderbuffers(1, &depthBufferAttachment);
        glBindRenderbuffer(GL_RENDERBUFFER, depthBufferAttachment);

        if (depthAttachment == ITextureFormat::itDepth32)  {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, device.getMaxSamplesCnt(), GL_DEPTH_COMPONENT32F, width, height);
        }

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferAttachment);
    }
    {
        auto frameBuffStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (frameBuffStatus != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: RenderBuff Framebuffer is not complete! error = " << frameBuffStatus
                      << std::endl;
    }
    //2. Fill texture framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_textureFbo);
    for (int i = 0; i < textureAttachments.size(); i++) {
        ((GTextureGL33 *)attachmentTextures[i].get())->bindToCurrentFrameBufferAsColor(i);
    }

    if (depthTexture != nullptr) {
        ((GTextureGL33 *) depthTexture.get())->bindToCurrentFrameBufferAsDepth();
    } else {

    }
    {
        auto frameBuffStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (frameBuffStatus != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete! error = " << frameBuffStatus << std::endl;
    }
}

GFrameBufferGL33::~GFrameBufferGL33() {
    glDeleteFramebuffers(1, &m_renderBufFbo);
    glDeleteFramebuffers(1, &m_textureFbo);

    for (int i = 0; i < renderBufferAttachments.size(); i++) {
        glDeleteRenderbuffers(1, &renderBufferAttachments[i]);
    }

    if (depthBufferAttachment > 0) {
        glDeleteRenderbuffers(1, &depthBufferAttachment);
    }
}

HGTexture GFrameBufferGL33::getAttachment(int index){
    return attachmentTextures[index];
}

HGTexture GFrameBufferGL33::getDepthTexture() {
    return depthTexture;
}

void GFrameBufferGL33::bindFrameBuffer(){
    glBindFramebuffer(GL_FRAMEBUFFER, m_renderBufFbo);
}

void GFrameBufferGL33::copyRenderBufferToTexture(){
    glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, m_renderBufFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, m_textureFbo);
    glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void GFrameBufferGL33::readRGBAPixels(int x, int y, int width, int height, void *data) {
    //Cant use m_renderBufFbo cause it uses multisampling
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_textureFbo);

//    ((GTextureGL33 *)attachmentTextures[0].get())->bindToCurrentFrameBufferAsDepth()

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels( x,y,  width, height,  GL_RGBA, GL_UNSIGNED_BYTE, data);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}
