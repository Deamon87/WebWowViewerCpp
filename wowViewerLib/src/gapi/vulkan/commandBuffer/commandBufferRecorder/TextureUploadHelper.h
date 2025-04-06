//
// Created by Deamon on 06.02.23.
//

#ifndef AWEBWOWVIEWERCPP_TEXTUREUPLOADHELPER_H
#define AWEBWOWVIEWERCPP_TEXTUREUPLOADHELPER_H

#include <vector>
#include "../../textures/GTextureVLK.h"
#include "CommandBufferRecorder.h"

struct TransitionParams {
    VkAccessFlags srcAccessMask;
    VkAccessFlags dstAccessMask;
    VkImageLayout oldLayout;
    VkImageLayout newLayout;
    uint32_t srcQueueFamilyIndex;
    uint32_t dstQueueFamilyIndex;
    VkPipelineStageFlags srcStageMask;
    VkPipelineStageFlags dstStageMask;
};


void transitionLayoutAndOwnageTextures(CmdBufRecorder &uploadCmdBufRecorder,
                                       const std::vector<std::weak_ptr<GTextureVLK>> &textures,
                                       const TransitionParams &transitionParams);

void textureUploadStrategy(const std::vector<std::weak_ptr<GTextureVLK>> &textures,
                           CmdBufRecorder &uploadCmdBufRecorder);


#endif //AWEBWOWVIEWERCPP_TEXTUREUPLOADHELPER_H
