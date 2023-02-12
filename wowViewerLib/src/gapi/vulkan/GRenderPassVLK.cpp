//
// Created by Deamon on 12/14/2020.
//

#include <stdexcept>
#include "GRenderPassVLK.h"


GRenderPassVLK::GRenderPassVLK(VkDevice vkDevice, std::vector<VkFormat> textureAttachments, VkFormat depthAttachmentFormat,
                               VkSampleCountFlagBits sampleCountBit, bool isSwapChainPass) {
    m_sampleCountBit = sampleCountBit;

    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkAttachmentDescription> colorAttachmentsResolves;

    std::vector<VkAttachmentReference> colorReferences;
    std::vector<VkAttachmentReference> colorResolveReferences;

    int attachmentIndex = 0;
    for (int i = 0; i < textureAttachments.size(); i++) {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = textureAttachments[i];
        colorAttachment.samples = sampleCountBit;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = isSwapChainPass ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        attachments.push_back(colorAttachment);
        if (attachments.size() == 1) {
            attachmentTypes.push_back(AttachmentType::atColor);
        } else {
            attachmentTypes.push_back(AttachmentType::atData);
        }

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = attachmentIndex++;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        colorReferences.push_back(colorAttachmentRef);

        //Add resolves if multisampling is on
        if (sampleCountBit != VK_SAMPLE_COUNT_1_BIT) {
            VkAttachmentDescription colorAttachmentResolve{};
            colorAttachmentResolve.format = textureAttachments[i];
            colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachmentResolve.finalLayout = isSwapChainPass ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            attachments.push_back(colorAttachmentResolve);
            attachmentTypes.push_back(AttachmentType::atData);

            VkAttachmentReference colorAttachmentResolveRef{};
            colorAttachmentResolveRef.attachment = attachmentIndex++;
            colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorResolveReferences.push_back(colorAttachmentResolveRef);
        }
    }

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = depthAttachmentFormat;
    depthAttachment.samples = sampleCountBit;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachments.push_back(depthAttachment);
    attachmentTypes.push_back(AttachmentType::atDepth);

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = attachmentIndex++;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = colorReferences.size();
    subpass.pColorAttachments = colorReferences.data();
    subpass.pResolveAttachments = colorResolveReferences.data();
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    std::vector<VkSubpassDependency> dependencies;
    if (isSwapChainPass) {
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        dependencies.push_back(dependency);
    } else {
        //Assume it's framebuffer pass
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies.push_back(dependency);

        dependency = {};
        dependency.srcSubpass = 0;
        dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies.push_back(dependency);
    }


    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = dependencies.size();
    renderPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(vkDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

VkSampleCountFlagBits GRenderPassVLK::getSampleCountBit() {
    return m_sampleCountBit;
}

VkRenderPass GRenderPassVLK::getRenderPass() {
    return renderPass;
}

std::vector<VkClearValue> GRenderPassVLK::produceClearColorVec(std::array<float, 3> colorClearColor, float depthClear) {
    std::vector<VkClearValue> result;
    for(int i = 0; i < attachmentTypes.size(); i++) {
        VkClearValue clearValue;
        if (attachmentTypes[i] == AttachmentType::atColor) {
            clearValue.color = {colorClearColor[0], colorClearColor[1], colorClearColor[2], 0.f};
        } else if (attachmentTypes[i] == AttachmentType::atData) {
            clearValue.color = {0,0,0,0};
        } else if (attachmentTypes[i] == AttachmentType::atDepth) {
            clearValue.depthStencil = {depthClear,0};
        }
        result.push_back(clearValue);
    }

    return result;
}
