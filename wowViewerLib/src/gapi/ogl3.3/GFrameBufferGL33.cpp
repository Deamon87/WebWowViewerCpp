//
// Created by Deamon on 14.05.2020.
//

#include "GFrameBufferGL33.h"


GFrameBufferGL33::GFrameBufferGL33 (
    IDevice &device,
    std::vector<ITextureFormat> textureAttachments,
    ITextureFormat depthAttachment,
    int width, int height) : mdevice(device) {

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

    glGenFramebuffers(1, &m_fbo);
    this->bindFrameBuffer();
    for (int i = 0; i < textureAttachments.size(); i++) {
        ((GTextureGL33 *)attachmentTextures[i].get())->bindToCurrentFrameBufferAsColor(i);
    }

    if (depthTexture != nullptr) {
        ((GTextureGL33 *) depthTexture.get())->bindToCurrentFrameBufferAsDepth();
    } else {

    }
    auto frameBuffStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if( frameBuffStatus != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete! error = " << frameBuffStatus <<  std::endl;
}

GFrameBufferGL33::~GFrameBufferGL33() {
    glDeleteFramebuffers(1, &m_fbo);
}

HGTexture GFrameBufferGL33::getAttachment(int index){
    return attachmentTextures[index];
}

HGTexture GFrameBufferGL33::getDepthTexture() {
    return depthTexture;
}

void GFrameBufferGL33::bindFrameBuffer(){
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}