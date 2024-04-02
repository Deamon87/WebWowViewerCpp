//
// Created by Deamon on 4/29/2023.
//

#ifndef AWEBWOWVIEWERCPP_GTEXTURESAMPLERVLK_H
#define AWEBWOWVIEWERCPP_GTEXTURESAMPLERVLK_H


#include "../context/vulkan_context.h"
#include "../../interface/textures/ITextureSampler.h"
#include "../IDeviceVulkan.h"

class GTextureSamplerVLK : public ITextureSampler {
public:
    GTextureSamplerVLK(IDeviceVulkan &deviceVlk, bool xWrapTex, bool yWrapTex, bool nearest);
    ~GTextureSamplerVLK() override;

    VkSampler getSampler() {
        return m_sampler;
    }
private:
    VkSampler m_sampler = VK_NULL_HANDLE;
    IDeviceVulkan &m_device;
};


#endif //AWEBWOWVIEWERCPP_GTEXTURESAMPLERVLK_H
