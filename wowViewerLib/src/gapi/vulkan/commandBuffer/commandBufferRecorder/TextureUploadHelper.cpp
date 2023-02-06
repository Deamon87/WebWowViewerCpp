//
// Created by Deamon on 06.02.23.
//

#include "TextureUploadHelper.h"

TextureUploadHelper::TextureUploadHelper(CmdBufRecorder &renderCmdBufRecorder, CmdBufRecorder &uploadCmdBufRecorder)
    : m_renderCmdBufRecorder(renderCmdBufRecorder), m_uploadCmdBufRecorder(uploadCmdBufRecorder)
{

}

TextureUploadHelper::~TextureUploadHelper() {
    // ------------------------------------
    // 1. Transition to upload queue
    // ------------------------------------

    std::vector<VkImageMemoryBarrier> imageMemoryBarriers;
    imageMemoryBarriers.reserve(m_texturesToUpload.size());

    for ( auto &textureVlk : m_texturesToUpload) {
        // Image memory barriers for the texture image
        VkImageMemoryBarrier &imageMemoryBarrier = imageMemoryBarriers.emplace_back();

        // The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
        VkImageSubresourceRange subresourceRange = {};
        // Image only contains color data
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // Start at first mip level
        subresourceRange.baseMipLevel = 0;
        // We will transition on all mip levels
        subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        // The 2D texture only has one layer
        subresourceRange.layerCount = 1;

        // Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
        imageMemoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        imageMemoryBarrier.subresourceRange = subresourceRange;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.image = textureVlk.texture.image;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = m_uploadCmdBufRecorder.getQueueFamily();
    }

    // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
    // Source pipeline stage is host write/read execution (VK_PIPELINE_STAGE_HOST_BIT)
    // Destination pipeline stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
    m_uploadCmdBufRecorder.recordPipelineBarrier(
        VK_PIPELINE_STAGE_HOST_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        imageMemoryBarriers
    );

    // 2. Do copy of texture to GPU memory
    for ( auto &textureVlk : m_texturesToUpload) {
        m_uploadCmdBufRecorder.copyBufferToImage(
            textureVlk.texture.image
        );
    }

}
