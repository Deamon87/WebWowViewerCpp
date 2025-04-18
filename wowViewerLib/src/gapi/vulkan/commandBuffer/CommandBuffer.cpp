//
// Created by Deamon on 05.02.23.
//

#include "CommandBuffer.h"


GCommandBuffer::GCommandBuffer(IDeviceVulkan &deviceVlk,
                               VkQueue vkQueue,
                               VkCommandPool commandPool,
                               bool isPrimary) : m_device(deviceVlk),
                                                            m_isPrimary(isPrimary),
                                                            m_commandPool(commandPool),
                                                            m_vkQueue(vkQueue)
{

}

CmdBufRecorder GCommandBuffer::beginRecord(const std::shared_ptr<GRenderPassVLK> &renderPass) {
    //If renderPass != nullptr -> it means this is a secondary command buffer, that needs to continue renderPass, that's going on outside
    if (m_isPrimary && renderPass != nullptr) {
        std::cerr << "tried to continue renderpass in primary buffer " << std::endl;
        throw std::runtime_error("tried to continue renderpass in primary buffer ");
    }

    createCommandBufVLK();

    return CmdBufRecorder(*this, renderPass);
}

void GCommandBuffer::createCommandBufVLK() {
    if (m_cmdBufWasCreated) {
//        //Dispose of previous buffer
//
//        auto l_deviceVlk = m_device.getVkDevice();
//        auto l_commandPool = m_commandPool;
//

        auto l_cmdBuf = m_cmdBuffer;

#ifdef LINK_TRACY
        if (m_isPrimary) {
            auto l_tracyContext = tracyContext;
            TracyVkCollect(l_tracyContext, l_cmdBuf);
        }
#endif
        vkResetCommandBuffer(m_cmdBuffer, 0);

    } else {

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = m_isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(m_device.getVkDevice(), &allocInfo, &m_cmdBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    if (!m_cmdBufWasCreated && m_isPrimary) {
#ifdef LINK_TRACY
        tracyContext = TracyVkContext(m_device.getVkPhysicalDevice(), m_device.getVkDevice(), m_vkQueue, m_cmdBuffer);
#endif
    }
    m_cmdBufWasCreated = true;
}
