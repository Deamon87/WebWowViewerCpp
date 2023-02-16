//
// Created by Deamon on 05.02.23.
//

#ifndef AWEBWOWVIEWERCPP_COMMANDBUFFER_H
#define AWEBWOWVIEWERCPP_COMMANDBUFFER_H


#include "../volk.h"
#include "../GDeviceVulkan.h"
#include "../GFrameBufferVLK.h"
#include "commandBufferRecorder/CommandBufferRecorder.h"

class GCommandBuffer {
public:
    friend class CmdBufRecorder;
    friend class RenderPassHelper;
    friend class IDeviceVulkan;

public:
    GCommandBuffer(IDeviceVulkan &deviceVlk, VkCommandPool commandPool, bool isPrimary, uint32_t queueFamilyIndex);

    CmdBufRecorder beginRecord(const std::shared_ptr<GRenderPassVLK> &renderPass);
    VkCommandBuffer getNativeCmdBuffer() {
        return m_cmdBuffer;
    }
private:
    void createCommandBufVLK();

private:
    IDeviceVulkan &m_device;
    VkCommandBuffer m_cmdBuffer;
    bool m_isPrimary = true;
    const uint32_t m_queueFamilyIndex;
    VkCommandPool m_commandPool;

    bool m_cmdBufWasCreated = false;
    bool m_hasData = false;
};


#endif //AWEBWOWVIEWERCPP_COMMANDBUFFER_H
