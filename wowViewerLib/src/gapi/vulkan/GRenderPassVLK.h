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

    const std::vector<VkFormat>& getColorAttachmentFormats() const {
        return m_colorAttachmentFormats;
    }
    VkFormat getDepthFormat() const {
        return m_depthFormat;
    }
    bool hasDepthAttachment() const {
        return m_depthFormat != VK_FORMAT_UNDEFINED;
    }

    // Attachment index accessors
    const std::vector<uint32_t>& getColorAttachmentIndices() const {
        return m_colorAttachmentIndices;
    }
    const std::vector<uint32_t>& getResolveAttachmentIndices() const {
        return m_resolveAttachmentIndices;
    }
    int32_t getDepthAttachmentIndex() const {
        return m_depthAttachmentIndex;
    }
    bool hasResolveAttachments() const {
        return !m_resolveAttachmentIndices.empty();
    }

private:
    VkSampleCountFlagBits m_sampleCountBit = VK_SAMPLE_COUNT_1_BIT;
    VkRenderPass renderPass = VK_NULL_HANDLE;

    uint32_t colorAttachmentCount = 0;

    bool m_invertZ = false;

    std::vector<VkFormat> m_colorAttachmentFormats;
    VkFormat m_depthFormat = VK_FORMAT_UNDEFINED;

    // Attachment indices for framebuffer creation
    std::vector<uint32_t> m_colorAttachmentIndices;
    std::vector<uint32_t> m_resolveAttachmentIndices;
    int32_t m_depthAttachmentIndex = -1;

    enum class AttachmentType {
        atColor,
        atData,
        atDepth
    };

    //Is used to fill proper clearColor vector
    std::vector<AttachmentType> attachmentTypes;

    void createRenderPass(const VkSampleCountFlagBits &sampleCountBit, bool isSwapChainPass,
                     VkDevice vkDevice,
                     const std::vector<VkFormat> &attachmentFormats,
                     const VkFormat &availableDepth,
                     bool clearColor,
                     bool clearDepth);
};


#endif //AWEBWOWVIEWERCPP_GRENDERPASSVLK_H
