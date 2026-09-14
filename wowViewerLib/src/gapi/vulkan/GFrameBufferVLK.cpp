//
// Created by Deamon on 12/11/2020.
//
#include <cstring>
#include "GFrameBufferVLK.h"
#include "textures/GTextureVLK.h"

#include "GRenderPassVLK.h"
#include "textures/GTextureSamplerVLK.h"

void GFrameBufferVLK::iterateOverAttachments(const std::vector<ITextureFormat> &textureAttachments, std::function<void(int i, VkFormat textureFormat)> callback) {
    for (int i = 0; i < textureAttachments.size(); i++) {
        VkFormat textureFormat;

        switch (textureAttachments[i]) {
            case ITextureFormat::itNone:
            case ITextureFormat::itDepth32:
                continue;

            case ITextureFormat::itRGBA:
                textureFormat = VK_FORMAT_R8G8B8A8_UNORM;
                break;

            case ITextureFormat::itRGBA16:
                textureFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
                break;

            case ITextureFormat::itInt:
                textureFormat = VK_FORMAT_R32_UINT;
                break;

            case ITextureFormat::itFloat32:
                textureFormat = VK_FORMAT_R32_SFLOAT;
                break;

            case ITextureFormat::itRGBAFloat32:
                textureFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
                break;
        }

        callback(i, textureFormat);
    }
}



inline void GFrameBufferVLK::initSampler(GDeviceVLK &device) {
    m_sampler = device.getSampler(false, false, true);
}

//Support for swapchain framebuffer
GFrameBufferVLK::GFrameBufferVLK(IDevice &device,
                                 const HGTexture &colorImage,
                                 const HGTexture &depthBuffer,
                                 int width, int height,
                                 const std::shared_ptr<GRenderPassVLK> &renderPass)
                                 : mdevice(dynamic_cast<GDeviceVLK &>(device)),
                                   m_renderPass(renderPass),
                                   m_attachmentTextures({colorImage}) {

    initSampler(mdevice);

    if (!depthBuffer)
    {
        // Find a suitable depth format
        VkFormat fbDepthFormat = mdevice.findDepthFormat();

        std::shared_ptr<GTextureVLK> h_depthTexture = std::make_shared<GTextureVLK>(
            mdevice,
            std::max<int>(width, 1), std::max<int>(height, 1),
            true,
            fbDepthFormat,
            VK_SAMPLE_COUNT_1_BIT,
            1,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        );

        m_depthTexture = h_depthTexture;

//        VkMemoryRequirements memoryRequirements;
//        vkGetImageMemoryRequirements(mdevice.getVkDevice(), std::dynamic_pointer_cast<GTextureVLK>(m_depthTexture)->texture.image, &memoryRequirements);
//        std::cout << "Depth Texture wants "
//                  << memoryRequirements.size
//                  << " bytes with "
//                  << memoryRequirements.alignment
//                  << " alignment. "
//                  << " Size allocated by VMA " << std::dynamic_pointer_cast<GTextureVLK>(m_depthTexture)->imageAllocationInfo.size
//                  << std::endl;
    } else {
        m_depthTexture = depthBuffer;
    }

    std::vector<VkImageView> attachments = {
        std::dynamic_pointer_cast<GTextureVLK>(colorImage)->texture.view,
    };
    if (m_depthTexture != nullptr)
        attachments.push_back(std::dynamic_pointer_cast<GTextureVLK>(m_depthTexture)->texture.view);

    VkFramebufferCreateInfo fbufCreateInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    fbufCreateInfo.pNext = nullptr;
    fbufCreateInfo.flags = 0;
    fbufCreateInfo.renderPass = m_renderPass->getRenderPass();
    fbufCreateInfo.attachmentCount = attachments.size();
    fbufCreateInfo.pAttachments = attachments.data();
    fbufCreateInfo.width = width;
    fbufCreateInfo.height = height;
    fbufCreateInfo.layers = 1;

    ERR_GUARD_VULKAN(vkCreateFramebuffer(mdevice.getVkDevice(), &fbufCreateInfo, nullptr, &m_frameBuffer));
}



GFrameBufferVLK::GFrameBufferVLK(IDevice &device,
                                 const std::shared_ptr<GRenderPassVLK> &renderPass,
                                 const HGTexture &depthBuffer,
                                 int width, int height)
    : mdevice(dynamic_cast<GDeviceVLK &>(device)),
      m_renderPass(renderPass),
      m_height(std::max<int>(height, 1)),
      m_width(std::max<int>(width, 1)) {

    initSampler(mdevice);

    const auto& colorFormats = renderPass->getColorAttachmentFormats();
    const auto& colorIndices = renderPass->getColorAttachmentIndices();
    const auto& resolveIndices = renderPass->getResolveAttachmentIndices();
    int32_t depthIndex = renderPass->getDepthAttachmentIndex();

    VkSampleCountFlagBits sampleCount = renderPass->getSampleCountBit();
    m_multiSampleCnt = (sampleCount == VK_SAMPLE_COUNT_1_BIT) ? 1 : static_cast<int>(sampleCount);

    // Calculate total attachment count
    size_t totalAttachments = colorIndices.size() + resolveIndices.size();
    if (depthIndex >= 0) {
        totalAttachments++;
    }

    // Pre-size attachments vector and texture storage
    std::vector<VkImageView> attachments(totalAttachments, VK_NULL_HANDLE);
    std::vector<std::shared_ptr<GTextureVLK>> colorTextures(colorFormats.size());
    std::vector<std::shared_ptr<GTextureVLK>> resolveTextures;
    if (renderPass->hasResolveAttachments()) {
        resolveTextures.resize(colorFormats.size());
    }

    // Create color attachments and place at correct indices
    for (size_t i = 0; i < colorFormats.size(); i++) {
        auto h_texture = std::make_shared<GTextureVLK>(
            mdevice,
            m_width, m_height,
            false,
            colorFormats[i],
            sampleCount,
            1,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
        );
        colorTextures[i] = h_texture;
        attachments[colorIndices[i]] = h_texture->texture.view;
        m_attachmentFormats.push_back(colorFormats[i]);
    }

    // Create resolve attachments if multisampling is enabled
    if (renderPass->hasResolveAttachments()) {
        for (size_t i = 0; i < colorFormats.size(); i++) {
            auto h_resolveTexture = std::make_shared<GTextureVLK>(
                mdevice,
                m_width, m_height,
                false,
                colorFormats[i],
                VK_SAMPLE_COUNT_1_BIT,
                1,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
            );
            resolveTextures[i] = h_resolveTexture;
            attachments[resolveIndices[i]] = h_resolveTexture->texture.view;
        }
    }

    // Store textures and create samplable wrappers
    for (size_t i = 0; i < colorTextures.size(); i++) {
        m_attachmentTextures.push_back(colorTextures[i]);
        m_colorAttachmentsSampled.push_back(std::make_shared<ISamplableTexture>(colorTextures[i], m_sampler));

        if (renderPass->hasResolveAttachments()) {
            m_attachmentTextures.push_back(resolveTextures[i]);
            m_resolveAttachmentsSampled.push_back(std::make_shared<ISamplableTexture>(resolveTextures[i], m_sampler));
        }
    }

    // Handle depth attachment
    if (depthIndex >= 0) {
        if (depthBuffer != nullptr) {
            m_depthTexture = depthBuffer;
            attachments[depthIndex] = std::dynamic_pointer_cast<GTextureVLK>(m_depthTexture)->texture.view;
        } else {
            VkFormat fbDepthFormat = renderPass->getDepthFormat();

            auto h_depthTexture = std::make_shared<GTextureVLK>(
                mdevice,
                m_width, m_height,
                true,
                fbDepthFormat,
                sampleCount,
                1,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
            );

            m_depthTexture = h_depthTexture;
            attachments[depthIndex] = h_depthTexture->texture.view;
        }
    }

    VkFramebufferCreateInfo fbufCreateInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    fbufCreateInfo.pNext = nullptr;
    fbufCreateInfo.flags = 0;
    fbufCreateInfo.renderPass = m_renderPass->getRenderPass();
    fbufCreateInfo.attachmentCount = attachments.size();
    fbufCreateInfo.pAttachments = attachments.data();
    fbufCreateInfo.width = m_width;
    fbufCreateInfo.height = m_height;
    fbufCreateInfo.layers = 1;

    ERR_GUARD_VULKAN(vkCreateFramebuffer(mdevice.getVkDevice(), &fbufCreateInfo, nullptr, &m_frameBuffer));
}

GFrameBufferVLK::~GFrameBufferVLK() {
    auto *l_device = &mdevice;
    auto l_frameBuffer = m_frameBuffer;
    mdevice.addDeallocationRecord([l_device, l_frameBuffer]() -> void {
        vkDestroyFramebuffer(l_device->getVkDevice(), l_frameBuffer, nullptr);
    });
}

void GFrameBufferVLK::readRGBAPixels(int x, int y, int width, int height, void *outputdata) {
    if (m_attachmentFormats.empty()) {
        //Cant read from swapchain framebuffer
        return;
    }

    int attachmentIndex = 0;

    VkFormat colorFormat = m_attachmentFormats[attachmentIndex];

    // Source for the copy is the last rendered swapchain image
    VkImage srcImage = std::dynamic_pointer_cast<GTextureVLK>(getAttachment(attachmentIndex)->getTexture())->texture.image;
    // std::cout << "srcImage: " << srcImage << std::endl;

    // Create the linear tiled destination image to copy to and to read the memory from
    VkImageCreateInfo imageCreateCI = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageCreateCI.imageType = VK_IMAGE_TYPE_2D;
    // Note that vkCmdBlitImage (if supported) will also do format conversions if the swapchain color format would differ
    imageCreateCI.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCreateCI.extent.width = width;
    imageCreateCI.extent.height = height;
    imageCreateCI.extent.depth = 1;
    imageCreateCI.arrayLayers = 1;
    imageCreateCI.mipLevels = 1;
    imageCreateCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateCI.tiling = VK_IMAGE_TILING_LINEAR;
    imageCreateCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    // Create the image
    VkImage dstImage;

    VmaAllocationCreateInfo allocImageCreateInfo = {};
    allocImageCreateInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;

    VmaAllocation imageAllocation = VK_NULL_HANDLE;
    VmaAllocationInfo imageAllocationInfo = {};

    vmaCreateImage(mdevice.getVMAAllocator(), &imageCreateCI, &allocImageCreateInfo, &dstImage,
                   &imageAllocation, &imageAllocationInfo);

    // Do the actual blit from the swapchain image to our host visible destination image
    mdevice.singleExecuteAndWait([&](VkCommandBuffer copyCmd) {
        // Transition destination image to transfer destination layout
        copyToImage(width, height, srcImage, colorFormat, dstImage, copyCmd);
    });


    // Get layout of the image (including row pitch)
    VkImageSubresource subResource { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
    VkSubresourceLayout subResourceLayout;
    vkGetImageSubresourceLayout(mdevice.getVkDevice(), dstImage, &subResource, &subResourceLayout);

    // Map image memory so we can start copying from it
    const char* data;
    vmaMapMemory(mdevice.getVMAAllocator(), imageAllocation,  (void**)&data);
    data += subResourceLayout.offset;

    //Flip image from vulkan
    int lineLenInBytes = 4 * width;
    if (lineLenInBytes != subResourceLayout.rowPitch) {
        lineLenInBytes = subResourceLayout.rowPitch;
    }
    for (int i = 0; i < height; i++) {
        std::memcpy((uint8_t *)outputdata + ((height - i - 1) * (4 * width)), (uint8_t *) data + (i *lineLenInBytes), (4 * width));
    }

    vmaUnmapMemory(mdevice.getVMAAllocator(), imageAllocation);
    vmaDestroyImage(mdevice.getVMAAllocator(), dstImage, imageAllocation);
}

void GFrameBufferVLK::copyToImage(int width, int height, VkImage srcImage, VkFormat inputColorFormat, VkImage dstImage,
                                  const VkCommandBuffer &copyCmd) const {// Transition destination image to transfer destination layout
    // Check blit support for source and destination
    VkFormatProperties formatProps;
    VkPhysicalDevice physicalDevice = mdevice.getVkPhysicalDevice();

    bool supportsBlit = false;

    // Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
    vkGetPhysicalDeviceFormatProperties(physicalDevice, inputColorFormat, &formatProps);
    if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
        std::cerr << "Device does not support blitting from optimal tiled images, using copy instead of blit!" << std::endl;
        supportsBlit = false;
    }

    // Check if the device supports blitting to linear images
    vkGetPhysicalDeviceFormatProperties(physicalDevice, VK_FORMAT_R8G8B8A8_UNORM, &formatProps);
    if (!(formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
        //std::cerr << "Device does not support blitting to linear tiled images, using copy instead of blit!" << std::endl;
        supportsBlit = false;
    }

    {
        VkImageSubresourceRange subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

        VkImageMemoryBarrier imageMemoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        imageMemoryBarrier.subresourceRange = subresourceRange;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.image = dstImage;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        vkCmdPipelineBarrier(
            copyCmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);
    }

    // Transition framebuffer image from shader read to transfer source layout
    {
        VkImageSubresourceRange subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

        VkImageMemoryBarrier imageMemoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        imageMemoryBarrier.subresourceRange = subresourceRange;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imageMemoryBarrier.image = srcImage;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        vkCmdPipelineBarrier(
            copyCmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);
    }

    // If source and destination support blit we'll blit as this also does automatic formt conversion (e.g. from BGR to RGB)a
    if (supportsBlit)
    {
        // Define the region to blit (we will blit the whole swapchain image)
        VkOffset3D blitSize;
        blitSize.x = width;
        blitSize.y = height;
        blitSize.z = 1;
        VkImageBlit imageBlitRegion{};
        imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.srcSubresource.layerCount = 1;
        imageBlitRegion.srcOffsets[1] = blitSize;
        imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.dstSubresource.layerCount = 1;
        imageBlitRegion.dstOffsets[1] = blitSize;

        // Issue the blit command
        vkCmdBlitImage(
            copyCmd,
            srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageBlitRegion,
            VK_FILTER_NEAREST);
    }
    else
    {
        // Otherwise use image copy (requires us to manually flip components)
        VkImageCopy imageCopyRegion{};
        imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.srcSubresource.layerCount = 1;
        imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.dstSubresource.layerCount = 1;
        imageCopyRegion.extent.width = width;
        imageCopyRegion.extent.height = height;
        imageCopyRegion.extent.depth = 1;

        // Issue the copy command
        vkCmdCopyImage(
            copyCmd,
            srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageCopyRegion);
    }

    // Transition destination image to general layout, which is the required layout for mapping the image memory later on
    {
        VkImageSubresourceRange subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

        VkImageMemoryBarrier imageMemoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        imageMemoryBarrier.subresourceRange = subresourceRange;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageMemoryBarrier.image = dstImage;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        vkCmdPipelineBarrier(
            copyCmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);
    }

    // Transition back the framebuff image after the blit\copy is done
    {
        VkImageSubresourceRange subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

        VkImageMemoryBarrier imageMemoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        imageMemoryBarrier.subresourceRange = subresourceRange;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageMemoryBarrier.image = srcImage;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        vkCmdPipelineBarrier(
            copyCmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);
    }
}

HGSamplableTexture GFrameBufferVLK::getAttachment(int index) {
    // Return resolve texture if available, otherwise return color texture
    auto idx = static_cast<size_t>(index);
    if (!m_resolveAttachmentsSampled.empty() && idx < m_resolveAttachmentsSampled.size()) {
        return m_resolveAttachmentsSampled[idx];
    }
    if (!m_colorAttachmentsSampled.empty() && idx < m_colorAttachmentsSampled.size()) {
        return m_colorAttachmentsSampled[idx];
    }

    return nullptr;
}

HGTexture GFrameBufferVLK::getDepthTexture() {
    return m_depthTexture;
}

void GFrameBufferVLK::bindFrameBuffer() {

}

void GFrameBufferVLK::copyRenderBufferToTexture() {

}
