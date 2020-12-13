//
// Created by Deamon on 12/11/2020.
//

#include "GFrameBufferVLK.h"
#include "textures/GTextureVLK.h"

void GFrameBufferVLK::iterateOverAttachments(std::vector<ITextureFormat> textureAttachments, std::function<void(int i, VkFormat textureFormat)> callback) {
    for (int i = 0; i < textureAttachments.size(); i++) {
        if (textureAttachments[i] == ITextureFormat::itNone) continue;
        if (textureAttachments[i] == ITextureFormat::itDepth32) continue;

        VkFormat textureFormat;
        if (textureAttachments[i] == ITextureFormat::itRGBA) {
            textureFormat = VK_FORMAT_R8G8B8A8_UNORM;
        } else if (textureAttachments[i] == ITextureFormat::itRGBAFloat32) {
            textureFormat = VK_FORMAT_R16G16B16_SFLOAT;
        }

        callback(i, textureFormat);
    }
}

GFrameBufferVLK::GFrameBufferVLK(IDevice &device, std::vector<ITextureFormat> textureAttachments,
                                 ITextureFormat depthAttachment,
                                 int width, int height) : mdevice(dynamic_cast<GDeviceVLK &>(device)), m_height(height), m_width(width) {


    std::vector<VkImageView> attachments;

    //Encapsulated for loop
    iterateOverAttachments(textureAttachments, [&](int i, VkFormat textureFormat) {
        std::shared_ptr<GTextureVLK> h_texture;
        h_texture.reset(new GTextureVLK(
            mdevice,
            width, height,
            false, false,
            false,
            textureFormat,
            1,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
        ));
        attachmentTextures.push_back(h_texture);

        attachments.push_back(h_texture->texture.view);
    });

    //Depth attachment
    {
        // Find a suitable depth format
        VkFormat fbDepthFormat = mdevice.findDepthFormat();

        std::shared_ptr<GTextureVLK> h_depthTexture;
        h_depthTexture.reset(new GTextureVLK(
            mdevice,
            width, height,
            false, false,
            true,
            fbDepthFormat,
            1,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        ));

        depthTexture = h_depthTexture;
        attachments.push_back(h_depthTexture->texture.view);
    }

    m_renderPass = mdevice.getRenderPass(textureAttachments, depthAttachment);

    VkFramebufferCreateInfo fbufCreateInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    fbufCreateInfo.pNext = nullptr;
    fbufCreateInfo.flags = 0;
    fbufCreateInfo.renderPass = m_renderPass;
    fbufCreateInfo.attachmentCount = attachments.size();
    fbufCreateInfo.pAttachments = attachments.data();
    fbufCreateInfo.width = width;
    fbufCreateInfo.height = height;
    fbufCreateInfo.layers = 1;

    ERR_GUARD_VULKAN(vkCreateFramebuffer(mdevice.getVkDevice(), &fbufCreateInfo, nullptr, &m_frameBuffer));
}

GFrameBufferVLK::~GFrameBufferVLK() {

}

void GFrameBufferVLK::readRGBAPixels(int x, int y, int width, int height, void *data) {

}

HGTexture GFrameBufferVLK::getAttachment(int index) {
    return attachmentTextures[index];
}

HGTexture GFrameBufferVLK::getDepthTexture() {
    return depthTexture;
}

void GFrameBufferVLK::bindFrameBuffer() {

}

void GFrameBufferVLK::copyRenderBufferToTexture() {

}
