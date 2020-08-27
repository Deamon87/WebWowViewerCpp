//
// Created by Deamon on 10/2/2019.
//

#ifndef AWEBWOWVIEWERCPP_GDESCRIPTORPOOLVLK_H
#define AWEBWOWVIEWERCPP_GDESCRIPTORPOOLVLK_H

class GDescriptorSets;

#include <vulkan/vulkan.h>
#include "../../interface/IDevice.h"
#include "../GDeviceVulkan.h"
#include "GDescriptorSet.h"

class GDescriptorPoolVLK {
public:
    explicit GDescriptorPoolVLK(IDevice &device);

    std::shared_ptr<GDescriptorSets> allocate(VkDescriptorSetLayout layout, int uniforms, int images);
    void deallocate(GDescriptorSets *set);

private:
    GDeviceVLK &m_device;
    VkDescriptorPool m_descriptorPool;

    int uniformsAvailable = 0;
    int imageAvailable = 0;
    int setsAvailable = 0;
};


#endif //AWEBWOWVIEWERCPP_GDESCRIPTORPOOLVLK_H
