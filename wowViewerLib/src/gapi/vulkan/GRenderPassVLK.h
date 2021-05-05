//
// Created by Deamon on 12/14/2020.
//

#ifndef AWEBWOWVIEWERCPP_GRENDERPASSVLK_H
#define AWEBWOWVIEWERCPP_GRENDERPASSVLK_H

#include "GDeviceVulkan.h"
#include <vector>

class GRenderPassVLK {
public:
    GRenderPassVLK(GDeviceVLK &device, std::vector<VkFormat> textureAttachments,
                   VkFormat depthAttachment, VkSampleCountFlagBits sampleCountBit,
                   bool isSwapChainPass);

    VkSampleCountFlagBits getSampleCountBit();
    VkRenderPass getRenderPass();

    std::vector<VkClearValue> produceClearColorVec(std::array<float,3> colorClearColor, float depthClear);

private:
    VkSampleCountFlagBits m_sampleCountBit;
    GDeviceVLK &mdevice;
    VkRenderPass renderPass;

    enum class AttachmentType {
        atColor,
        atData,
        atDepth
    };

    //Is used to fill proper clearColor vector
    std::vector<AttachmentType> attachmentTypes;

};


#endif //AWEBWOWVIEWERCPP_GRENDERPASSVLK_H
