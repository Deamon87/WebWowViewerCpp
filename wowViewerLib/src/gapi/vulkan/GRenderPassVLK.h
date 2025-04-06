//
// Created by Deamon on 12/14/2020.
//

#ifndef AWEBWOWVIEWERCPP_GRENDERPASSVLK_H
#define AWEBWOWVIEWERCPP_GRENDERPASSVLK_H

#include "context/vulkan_context.h"
#include "../interface/textures/ITexture.h"
#include "../interface/IDevice.h"
#include <vector>
#include <array>

class GRenderPassVLK {
public:
    GRenderPassVLK(IDevice &device,
                   const std::vector<ITextureFormat> &textureAttachments,
                   ITextureFormat depthAttachment,
                   VkSampleCountFlagBits sampleCountBit,
                   bool invertZ,
                   bool isSwapChainPass,
                   bool clearColor,
                   bool clearDepth);

    GRenderPassVLK(VkDevice vkDevice,
                   const std::vector<VkFormat> &textureAttachments,
                   VkFormat depthAttachment,
                   VkSampleCountFlagBits sampleCountBit,
                   bool invertZ,
                   bool isSwapChainPass,
                   bool clearColor,
                   bool clearDepth);

    VkSampleCountFlagBits getSampleCountBit() const;
    uint8_t getColorAttachmentsCount() const;
    VkRenderPass getRenderPass() const;

    std::vector<VkClearValue> produceClearColorVec(std::array<float,3> colorClearColor);
    bool getInvertZ() const {
        return m_invertZ;
    }

private:
    VkSampleCountFlagBits m_sampleCountBit;
    VkRenderPass renderPass;

    uint32_t colorAttachmentCount = 0;

    bool m_invertZ = false;

    enum class AttachmentType {
        atColor,
        atData,
        atDepth
    };

    //Is used to fill proper clearColor vector
    std::vector<AttachmentType> attachmentTypes;

    void createRenderPass(const ITextureFormat &depthAttachmentFormat,
                     const VkSampleCountFlagBits &sampleCountBit, bool isSwapChainPass,
                     VkDevice vkDevice,
                     const std::vector<VkFormat> &attachmentFormats,
                     const VkFormat &availableDepth,
                     bool clearColor,
                     bool clearDepth);
};


#endif //AWEBWOWVIEWERCPP_GRENDERPASSVLK_H
