//
// Created by Deamon on 12.12.22.
//

#ifndef AWEBWOWVIEWERCPP_FRONTENDUIRENDERFORWARDVLK_H
#define AWEBWOWVIEWERCPP_FRONTENDUIRENDERFORWARDVLK_H


#include <queue>
#include "../FrontendUIRenderer.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/GDeviceVulkan.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/buffers/GBufferVLK.h"

class FrontendUIRenderForwardVLK : public FrontendUIRenderer {
public:
    explicit FrontendUIRenderForwardVLK(HGDeviceVLK hDevice);
    void putIntoQueue(std::shared_ptr<FrontendUIInputParams> &frameInputParams) override;

    void update(VkCommandBuffer udBuffer, VkCommandBuffer swapChainDraw);
private:
    HGDeviceVLK m_device;

    //Frame counter
    int m_frame = 0;

    //Rendering pipelining
    std::mutex m_inputParamsMtx;
    std::queue<std::shared_ptr<FrontendUIInputParams>> m_inputParams;
    std::queue<std::shared_ptr<FrontendUIInputParams>> m_updateParams;

private:
    HGBufferVLK vboBuffer;
    HGBufferVLK iboBuffer;
    HGBufferVLK uboBuffer;

    void createBuffers();

};


#endif //AWEBWOWVIEWERCPP_FRONTENDUIRENDERFORWARDVLK_H
