//
// Created by Deamon on 05.02.23.
//

#ifndef AWEBWOWVIEWERCPP_COMMANDBUFFER_H
#define AWEBWOWVIEWERCPP_COMMANDBUFFER_H


#include "../volk.h"
#include "../GDeviceVulkan.h"
#include "../GFrameBufferVLK.h"
#include "commandBufferRecorder/CommandBufferRecorder.h"
#include "../../../renderer/frame/FrameProfile.h"

class GCommandBuffer {
public:
    friend class CmdBufRecorder;
    friend class RenderPassHelper;
    friend class RenderPassHelper;
    friend class IDeviceVulkan;

public:
    GCommandBuffer(IDeviceVulkan &deviceVlk, VkQueue vkQueue, VkCommandPool commandPool, bool isPrimary);

    CmdBufRecorder beginRecord(const std::shared_ptr<GRenderPassVLK> &renderPass);
    VkCommandBuffer getNativeCmdBuffer() const {
        return m_cmdBuffer;
    }
private:
    void createCommandBufVLK();

private:
    IDeviceVulkan &m_device;
    VkCommandBuffer m_cmdBuffer;
    VkQueue m_vkQueue;
    bool m_isPrimary = true;
    VkCommandPool m_commandPool;

    bool m_cmdBufWasCreated = false;
    bool m_hasData = false;

#ifdef LINK_TRACY
    TracyVkCtx tracyContext;
#endif
};


#endif //AWEBWOWVIEWERCPP_COMMANDBUFFER_H
