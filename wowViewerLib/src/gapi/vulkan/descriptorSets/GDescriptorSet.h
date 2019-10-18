//
// Created by Deamon on 10/2/2019.
//

#ifndef AWEBWOWVIEWERCPP_GDESCRIPTORSET_H
#define AWEBWOWVIEWERCPP_GDESCRIPTORSET_H

class GDeviceVLK;

#include <vector>
#include <vulkan/vulkan.h>
#include "../../interface/IDevice.h"
#include "../GDeviceVulkan.h"



class GDescriptorSets {
public:
    explicit GDescriptorSets(IDevice &device, VkDescriptorSet descriptorSet);

    void writeToDescriptorSets(std::vector<VkWriteDescriptorSet> &descriptorWrites);

    VkDescriptorSet getDescSet() {return m_descriptorSet;}
private:
    GDeviceVLK &m_device;
    VkDescriptorSet m_descriptorSet;
};


#endif //AWEBWOWVIEWERCPP_GDESCRIPTORSET_H
