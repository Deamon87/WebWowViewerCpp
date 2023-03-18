//
// Created by Deamon on 06.02.23.
//

#include "TextureUploadHelper.h"

void transitionLayoutAndOwnageTextures(CmdBufRecorder &uploadCmdBufRecorder,
                                       const std::vector<std::weak_ptr<GTextureVLK>> &textures,
                                       const TransitionParams &transitionParams) {
    std::vector<VkImageMemoryBarrier> imageMemoryBarriers;
    imageMemoryBarriers.reserve(textures.size());

    for ( auto &wtextureVlk : textures) {
        if (wtextureVlk.expired()) continue;
        auto textureVlk = wtextureVlk.lock();

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
        imageMemoryBarrier.srcAccessMask = transitionParams.srcAccessMask;
        imageMemoryBarrier.dstAccessMask = transitionParams.dstAccessMask;
        imageMemoryBarrier.oldLayout = transitionParams.oldLayout;
        imageMemoryBarrier.newLayout = transitionParams.newLayout;
        imageMemoryBarrier.image = textureVlk->texture.image;
        imageMemoryBarrier.srcQueueFamilyIndex = transitionParams.srcQueueFamilyIndex;
        imageMemoryBarrier.dstQueueFamilyIndex = transitionParams.dstQueueFamilyIndex;
    }

    // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
// Source pipeline stage is host write/read execution (VK_PIPELINE_STAGE_HOST_BIT)
// Destination pipeline stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
    uploadCmdBufRecorder.recordPipelineBarrier(
        transitionParams.srcStageMask,
        transitionParams.dstStageMask,
        imageMemoryBarriers
    );
}

void textureUploadStrategy(const std::vector<std::weak_ptr<GTextureVLK>> &textures, CmdBufRecorder &renderCmdBufRecorder, CmdBufRecorder &uploadCmdBufRecorder) {
    if (textures.empty()) return;

    // ------------------------------------
    // 1. Transition ownership to upload queue
    // ------------------------------------

    {
        const TransitionParams transitionParams = {
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .srcStageMask = VK_PIPELINE_STAGE_HOST_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT
        };

//        transitionLayoutAndOwnageTextures(renderCmdBufRecorder, textures, transitionParams);
        transitionLayoutAndOwnageTextures(uploadCmdBufRecorder, textures, transitionParams);
    }

    // ------------------------------------
    // 2. Do copy of texture to GPU memory
    // ------------------------------------

    for ( auto &wtextureVlk : textures) {
        if (wtextureVlk.expired()) continue;
        auto textureVlk = wtextureVlk.lock();

        auto updateRecord = textureVlk->getAndPlanDestroy();

        uploadCmdBufRecorder.copyBufferToImage(
            updateRecord->stagingBuffer,
            textureVlk->texture.image,
            updateRecord->bufferCopyRegions
        );
    }
//
//    // --------------------------------------------------------
//    // 3. Transition ownership from upload queue to render queue
//    // --------------------------------------------------------
//    {
//        if (uploadCmdBufRecorder.getQueueFamily() == renderCmdBufRecorder.getQueueFamily()) {
//            TransitionParams transitionParams = {
//                .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
//                .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
//                .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//                .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//                .srcQueueFamilyIndex = uploadCmdBufRecorder.getQueueFamily(),
//                .dstQueueFamilyIndex = renderCmdBufRecorder.getQueueFamily(),
//                .srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
//                .dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
//            };
//
//            transitionLayoutAndOwnageTextures(uploadCmdBufRecorder, textures, transitionParams);
//        } else {
            //Change access mask
            TransitionParams transitionParams = {
                .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                .dstAccessMask = 0,
                .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT
            };

            transitionLayoutAndOwnageTextures(uploadCmdBufRecorder, textures, transitionParams);
//        }
//
//        //Change ownership
//        {
//             TransitionParams transitionParams = {
//                .srcAccessMask = 0,
//                .dstAccessMask = 0,
//                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
//                .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//                .srcQueueFamilyIndex = uploadCmdBufRecorder.getQueueFamily(),
//                .dstQueueFamilyIndex = renderCmdBufRecorder.getQueueFamily(),
//                .srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
//                .dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT
//            };
//
//            transitionLayoutAndOwnageTextures(uploadCmdBufRecorder, textures, transitionParams);
//            transitionLayoutAndOwnageTextures(renderCmdBufRecorder, textures, transitionParams);
//        }

       /* {
            TransitionParams transitionParams = {
                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .srcQueueFamilyIndex = renderCmdBufRecorder.getQueueFamily(),
                .dstQueueFamilyIndex = renderCmdBufRecorder.getQueueFamily(),
                .srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
            };
            transitionLayoutAndOwnageTextures(renderCmdBufRecorder, textures, transitionParams);
        }*/
    //}
}
