//
// Created by Deamon on 14.05.2020.
//

#include "GFrameBufferGL33.h"


GFrameBufferGL33::GFrameBufferGL33 (
    const HGDevice &device,
    std::vector<ITextureFormat> textureAttachments,
    ITextureFormat depthAttachment,
    int width, int height) : mdevice(device), m_height(height), m_width(width) {

    m_depthAttachment = depthAttachment;
    attachmentTextures = std::vector<HGTexture>(textureAttachments.size());
    for (int i = 0; i < textureAttachments.size(); i++) {
        if (textureAttachments[i] != ITextureFormat::itNone) {
            attachmentTextures[i] = mdevice->createTexture(false, false);
            attachmentTextures[i]->loadData(width, height, nullptr, textureAttachments[i]);
        }
    }
    if (depthAttachment != ITextureFormat::itNone) {
        depthTexture = mdevice->createTexture(false, false);
        depthTexture->loadData(width, height, nullptr, depthAttachment);
    }

    defaultTextureForRenderBufFBO = mdevice->createTexture(false, false);
    defaultTextureForRenderBufFBO->loadData(width, height, nullptr, ITextureFormat::itRGBA);

    glGenFramebuffers(+1, &m_renderBufFbo);
    glGenFramebuffers(+1, &m_textureFbo);

    //Create m_renderBufFbo
    this->bindFrameBuffer();
    //1.1 To make OGL ES happy
    ((GTextureGL33 *)defaultTextureForRenderBufFBO.get())->bindToCurrentFrameBufferAsColor(0);

    //1.2 First fill framebuffer with renderbuffers
    renderBufferAttachments = std::vector<GLuint>(textureAttachments.size());
    for (int i = 0; i < renderBufferAttachments.size(); i++) {
        if (textureAttachments[i] == ITextureFormat::itDepth32) continue;

        glGenRenderbuffers(1, &renderBufferAttachments[i]);
        glBindRenderbuffer(GL_RENDERBUFFER, renderBufferAttachments[i]);

        if (textureAttachments[i] == ITextureFormat::itRGBA) {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, device->getMaxSamplesCnt(), GL_RGBA8, width, height);
        } else if (textureAttachments[i] == ITextureFormat::itRGBAFloat32)  {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, device->getMaxSamplesCnt(), GL_RGBA32F, width, height);
        }
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_RENDERBUFFER, renderBufferAttachments[i]);
    }
    if (depthAttachment != ITextureFormat::itNone) {
        glGenRenderbuffers(1, &depthBufferAttachment);
        glBindRenderbuffer(GL_RENDERBUFFER, depthBufferAttachment);

        if (depthAttachment == ITextureFormat::itDepth32)  {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, device->getMaxSamplesCnt(), GL_DEPTH32F_STENCIL8, width, height);
        }
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBufferAttachment);
    }
//    {
//        auto frameBuffStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
//        if (frameBuffStatus != GL_FRAMEBUFFER_COMPLETE)
//            std::cout << "ERROR::FRAMEBUFFER:: RenderBuff Framebuffer is not complete! error = " << frameBuffStatus
//                      << std::endl;
//    }
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
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    frame_created = device->getFrameNumber();
}

GFrameBufferGL33::~GFrameBufferGL33() {
//    glBindFramebuffer(GL_FRAMEBUFFER, m_renderBufFbo);
//
//    std::vector<GLenum> discards;
//    for (int i = 0; i < renderBufferAttachments.size(); i++) {
//        discards.push_back(GL_COLOR_ATTACHMENT0+i);
//    }
//    if (m_depthAttachment != ITextureFormat::itNone) {
//        discards.push_back(GL_DEPTH_STENCIL_ATTACHMENT);
//    }
//    glDiscardFramebufferEXT(GL_FRAMEBUFFER,discards.size(),discards.data());

    frame_destroyed = mdevice->getFrameNumber();

//    std::cout << "m_textureFbo = " << m_textureFbo << " frame_created = " << frame_created << " " <<
//        "frame_destroyed = " << frame_destroyed << std::endl;

    auto l_textureFbo = m_textureFbo;
    auto l_renderBufferAttachments = renderBufferAttachments;
    auto l_depthBufferAttachment = depthBufferAttachment;
    auto l_renderBufFbo = depthBufferAttachment;
    mdevice->addDeallocationRecord([l_textureFbo, l_renderBufferAttachments, l_depthBufferAttachment, l_renderBufFbo]() {
        glDeleteFramebuffers(1, &l_textureFbo);
        glBindFramebuffer(GL_FRAMEBUFFER, l_renderBufFbo);
        for (int i = 0; i < l_renderBufferAttachments.size(); i++) {
            glBindRenderbuffer(GL_RENDERBUFFER, l_renderBufferAttachments[i]);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, 0, GL_RGBA8, 0, 0);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glDeleteRenderbuffers(1, &l_renderBufferAttachments[i]);
        }

        if (l_depthBufferAttachment > 0) {
            glBindRenderbuffer(GL_RENDERBUFFER, l_depthBufferAttachment);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, 0, GL_DEPTH32F_STENCIL8, 0, 0);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glDeleteRenderbuffers(1, &l_depthBufferAttachment);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &l_renderBufFbo);
    });
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
//    std::cout << "m_textureFbo = " << m_textureFbo << " called on " << mdevice->getFrameNumber()
//              << std::endl;

    logGLError
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_renderBufFbo);
    logGLError
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_textureFbo);

    logGLError
    glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    logGLError
//    glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, attachmentCount, attachments)
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void GFrameBufferGL33::readRGBAPixels(int x, int y, int width, int height, void *data) {
    //Cant use m_renderBufFbo cause it uses multisampling
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_textureFbo);

//    ((GTextureGL33 *)attachmentTextures[0].get())->bindToCurrentFrameBufferAsDepth()

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels( x,y,  width, height,  GL_RGBA, GL_UNSIGNED_BYTE, data);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}
