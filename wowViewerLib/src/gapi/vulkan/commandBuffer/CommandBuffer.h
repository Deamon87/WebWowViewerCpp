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
    friend CmdBufRecorder;
    friend RenderPassHelper;

public:
    GCommandBuffer(GDeviceVLK &deviceVlk, VkCommandPool commandPool, bool isPrimary, uint32_t queueFamilyIndex);

    CmdBufRecorder beginRecord(const std::shared_ptr<GRenderPassVLK> &renderPass);
private:
    VkCommandBuffer m_cmdBuffer;
    bool m_isPrimary = true;

    const uint32_t m_queueFamilyIndex;
};


#endif //AWEBWOWVIEWERCPP_COMMANDBUFFER_H
