//
// Created by Deamon on 12/14/2020.
//

#ifndef AWEBWOWVIEWERCPP_GRENDERPASSVLK_H
#define AWEBWOWVIEWERCPP_GRENDERPASSVLK_H

#include "context/vulkan_context.h"
#include <vector>
#include <array>

class GRenderPassVLK {
public:
    GRenderPassVLK(VkDevice vkDevice,
                   std::vector<VkFormat> textureAttachments,
                   VkFormat depthAttachment,
                   VkSampleCountFlagBits sampleCountBit,
                   bool invertZ,
                   bool isSwapChainPass);

    VkSampleCountFlagBits getSampleCountBit();
    VkRenderPass getRenderPass();

    std::vector<VkClearValue> produceClearColorVec(std::array<float,3> colorClearColor, float depthClear);
    bool getInvertZ() {
        return m_invertZ;
    }

private:
    VkSampleCountFlagBits m_sampleCountBit;
    VkRenderPass renderPass;

    bool m_invertZ = false;

    enum class AttachmentType {
        atColor,
        atData,
        atDepth
    };

    //Is used to fill proper clearColor vector
    std::vector<AttachmentType> attachmentTypes;

};


#endif //AWEBWOWVIEWERCPP_GRENDERPASSVLK_H
