//
// Created by Deamon on 12/11/2020.
//

#include "GFrameBufferVLK.h"
#include "textures/GTextureVLK.h"

GFrameBufferVLK::GFrameBufferVLK(IDevice &device, std::vector<ITextureFormat> textureAttachments,
                                 ITextureFormat depthAttachment,
                                 int width, int height) : mdevice(dynamic_cast<GDeviceVLK &>(device)), m_height(height), m_width(width) {

    std::vector<VkAttachmentDescription> attachmentDescriptions = {};
    std::vector<VkAttachmentReference> colorReferences;
    std::vector<VkImageView> attachments;


    uint32_t colorRefIndex = 0;
    for (int i = 0; i < textureAttachments.size(); i++) {
        if (textureAttachments[i] == ITextureFormat::itNone) continue;
        if (textureAttachments[i] == ITextureFormat::itDepth32) continue;

        VkFormat textureFormat;
        if (textureAttachments[i] == ITextureFormat::itRGBA) {
            textureFormat = VK_FORMAT_R8G8B8A8_UNORM;
        } else if (textureAttachments[i] == ITextureFormat::itRGBAFloat32) {
            textureFormat = VK_FORMAT_R16G16B16_SFLOAT;
        }

        std::shared_ptr<GTextureVLK> h_texture;
        h_texture.reset(new GTextureVLK(
            mdevice,
            width, height,
            false, false,
            false,
            textureFormat,
            1,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
        ));
        attachmentTextures.push_back(h_texture);

        VkAttachmentDescription colorAttachment;
        colorAttachment.format = textureFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        attachmentDescriptions.push_back(colorAttachment);
        colorReferences.push_back({ colorRefIndex++, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
        attachments.push_back(h_texture->texture.view);
    }

    //Depth attachment
    {
        // Find a suitable depth format
        VkFormat fbDepthFormat = mdevice.findDepthFormat();

        // Depth attachment
        VkAttachmentDescription depthAttachmentDesc;
        depthAttachmentDesc.format = fbDepthFormat;
        depthAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        attachmentDescriptions.push_back(depthAttachmentDesc);


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
    VkAttachmentReference depthReference = { static_cast<uint32_t>(colorReferences.size()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };


    // Create a separate render pass for the offscreen rendering as it may differ from the one used for scene rendering
    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = colorReferences.size();
    subpassDescription.pColorAttachments = colorReferences.data();
    subpassDescription.pDepthStencilAttachment = &depthReference;

    // Use subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // Create the actual renderpass
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
    renderPassInfo.pAttachments = attachmentDescriptions.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    ERR_GUARD_VULKAN(vkCreateRenderPass(mdevice.getVkDevice(), &renderPassInfo, nullptr, &m_renderPass));

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
