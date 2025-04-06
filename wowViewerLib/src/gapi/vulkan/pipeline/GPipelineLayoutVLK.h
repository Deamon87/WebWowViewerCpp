//
// Created by Deamon on 10/17/2023.
//

#ifndef AWEBWOWVIEWERCPP_GPIPELINELAYOUTVLK_H
#define AWEBWOWVIEWERCPP_GPIPELINELAYOUTVLK_H

#include "../../interface/IDevice.h"
#include "../GDeviceVulkan.h"

class GPipelineLayoutVLK {
public:
    GPipelineLayoutVLK(IDevice &device, VkPipelineLayout layout);;
    ~GPipelineLayoutVLK();

    VkPipelineLayout getLayout() {return m_layout;}
private:
    GDeviceVLK &m_device;
    VkPipelineLayout m_layout;
};


#endif //AWEBWOWVIEWERCPP_GPIPELINELAYOUTVLK_H
