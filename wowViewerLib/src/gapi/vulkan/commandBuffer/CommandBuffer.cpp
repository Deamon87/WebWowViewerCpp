//
// Created by Deamon on 05.02.23.
//

#include "CommandBuffer.h"
#include "../GRenderPassVLK.h"
#include "../GFrameBufferVLK.h"
#include "../GPipelineVLK.h"

GCommandBuffer::GCommandBuffer(GDeviceVLK &deviceVlk, VkCommandPool commandPool, bool isPrimary, uint32_t queueFamilyIndex) : m_queueFamilyIndex(queueFamilyIndex) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(deviceVlk.getVkDevice(), &allocInfo, &m_cmdBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

CmdBufRecorder GCommandBuffer::beginRecord(const std::shared_ptr<GRenderPassVLK> &renderPass) {
    //If renderPass != nullptr -> it means this is a secondary command buffer, that needs to continue renderPass, that's going on outside
    if (m_isPrimary && renderPass != nullptr) {
        std::cerr << "tried to continue renderpass in primary buffer " << std::endl;
        throw std::runtime_error("tried to continue renderpass in primary buffer ");
    }

    return CmdBufRecorder(*this, renderPass);
}
