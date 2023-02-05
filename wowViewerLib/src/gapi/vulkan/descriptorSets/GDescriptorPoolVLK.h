//
// Created by Deamon on 10/2/2019.
//

#ifndef AWEBWOWVIEWERCPP_GDESCRIPTORPOOLVLK_H
#define AWEBWOWVIEWERCPP_GDESCRIPTORPOOLVLK_H

class GDescriptorSet;

#include "../../interface/IDevice.h"
#include "../GDeviceVulkan.h"
#include "GDescriptorSet.h"
#include "GDescriptorSetLayout.h"

class GDescriptorPoolVLK : public std::enable_shared_from_this<GDescriptorPoolVLK>{
public:
    explicit GDescriptorPoolVLK(IDevice &device);

    VkDescriptorSet allocate(const std::shared_ptr<GDescriptorSetLayout> &gDescriptorSetLayout);
    void deallocate(const std::shared_ptr<GDescriptorSetLayout> &hDescriptorLayout, VkDescriptorSet descSet);

private:
    GDeviceVLK &m_device;
    VkDescriptorPool m_descriptorPool;

    int uniformsAvailable = 0;
    int imageAvailable = 0;
    int setsAvailable = 0;
};


#endif //AWEBWOWVIEWERCPP_GDESCRIPTORPOOLVLK_H
