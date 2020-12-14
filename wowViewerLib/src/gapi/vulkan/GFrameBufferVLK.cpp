//
// Created by Deamon on 12/11/2020.
//

#include <cstring>
#include "GFrameBufferVLK.h"
#include "textures/GTextureVLK.h"
#include "GRenderPassVLK.h"

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

    VkSampleCountFlagBits sampleBit = mdevice.getMaxSamplesBit();
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
            sampleBit,
            1,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
        ));
        attachmentTextures.push_back(h_texture);
        attachments.push_back(h_texture->texture.view);
        attachmentFormats.push_back(textureFormat);

        if (mdevice.getMaxSamplesBit() != VK_SAMPLE_COUNT_1_BIT) {
            std::shared_ptr<GTextureVLK> h_texture;
            h_texture.reset(new GTextureVLK(
                mdevice,
                width, height,
                false, false,
                false,
                textureFormat,
                VK_SAMPLE_COUNT_1_BIT,
                1,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
            ));
            attachmentTextures.push_back(h_texture);
            attachments.push_back(h_texture->texture.view);
        }


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
            sampleBit,
            1,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        ));

        depthTexture = h_depthTexture;
        attachments.push_back(h_depthTexture->texture.view);
    }

    m_renderPass = mdevice.getRenderPass(textureAttachments, depthAttachment, mdevice.getMaxSamplesBit(), false);

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

GFrameBufferVLK::~GFrameBufferVLK() {

}

void GFrameBufferVLK::readRGBAPixels(int x, int y, int width, int height, void *outputdata) {
// Check blit support for source and destination
    VkFormatProperties formatProps;

    VkPhysicalDevice physicalDevice = mdevice.getVkPhysicalDevice();

    int attachmentIndex = 0;

    VkFormat colorFormat = attachmentFormats[attachmentIndex];
    bool supportsBlit = false;

    // Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
    vkGetPhysicalDeviceFormatProperties(physicalDevice, colorFormat, &formatProps);
    if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
        std::cerr << "Device does not support blitting from optimal tiled images, using copy instead of blit!" << std::endl;
        supportsBlit = false;
    }

    // Check if the device supports blitting to linear images
    vkGetPhysicalDeviceFormatProperties(physicalDevice, VK_FORMAT_R8G8B8A8_UNORM, &formatProps);
    if (!(formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
        std::cerr << "Device does not support blitting to linear tiled images, using copy instead of blit!" << std::endl;
        supportsBlit = false;
    }

    // Source for the copy is the last rendered swapchain image
    VkImage srcImage = ((GTextureVLK *)(getAttachment(attachmentIndex).get()))->texture.image;

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

        // Transition swapchain image from present to transfer source layout
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

        // If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB)
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

        // Transition back the swap chain image after the blit is done
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
    });


    // Get layout of the image (including row pitch)
    VkImageSubresource subResource { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
    VkSubresourceLayout subResourceLayout;
    vkGetImageSubresourceLayout(mdevice.getVkDevice(), dstImage, &subResource, &subResourceLayout);

    // Map image memory so we can start copying from it
    const char* data;
    vmaMapMemory(mdevice.getVMAAllocator(), imageAllocation,  (void**)&data);
    data += subResourceLayout.offset;

    // If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
    bool colorSwizzle = false;
    // Check if source is BGR
    // Note: Not complete, only contains most common and basic BGR surface formats for demonstration purposes
    if (!supportsBlit)
    {
        std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
        colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), colorFormat) != formatsBGR.end());
    }

    //Flip image from vulkan
    for (int i = 0; i < height; i++) {
        std::memcpy((uint8_t *)outputdata + ((height - i - 1) *4 * width), (uint8_t *) data + (i *4 * width), (4 * width));
    }

    vmaUnmapMemory(mdevice.getVMAAllocator(), imageAllocation);
    vmaDestroyImage(mdevice.getVMAAllocator(), dstImage, imageAllocation);
}

HGTexture GFrameBufferVLK::getAttachment(int index) {
    //If multisampling is active - return only resolved textures
    if (mdevice.getMaxSamplesBit() != VK_SAMPLE_COUNT_1_BIT) {
        index = 2*index+1;
    }

    return attachmentTextures[index];
}

HGTexture GFrameBufferVLK::getDepthTexture() {
    return depthTexture;
}

void GFrameBufferVLK::bindFrameBuffer() {

}

void GFrameBufferVLK::copyRenderBufferToTexture() {

}
