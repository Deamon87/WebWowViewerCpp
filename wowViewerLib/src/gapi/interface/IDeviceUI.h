//
// Created by deamon on 20.12.19.
//

#ifndef AWEBWOWVIEWERCPP_IDEVICEUI_H
#define AWEBWOWVIEWERCPP_IDEVICEUI_H

#include "IDevice.h"

class IDeviceUI {
public:
    virtual void renderUI() = 0;

    std::vector<HGTexture> requiredTextures;

    #ifdef LINK_VULKAN
    virtual void renderUIVLK(VkCommandBuffer commandBuffer) = 0;
    #endif
};

#endif //AWEBWOWVIEWERCPP_IDEVICEUI_H
