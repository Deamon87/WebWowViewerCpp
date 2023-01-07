//
// Created by Deamon on 10/2/2019.
//

#ifndef AWEBWOWVIEWERCPP_GDESCRIPTORSET_H
#define AWEBWOWVIEWERCPP_GDESCRIPTORSET_H

class GDeviceVLK;
class GDescriptorPoolVLK;

#include <vector>
#include "../../interface/IDevice.h"
#include "../GDeviceVulkan.h"



class GDescriptorSets {
public:
    explicit GDescriptorSets(IDevice &device, VkDescriptorSet descriptorSet, GDescriptorPoolVLK* parentPool, int uniforms, int images);
    ~GDescriptorSets();

    void writeToDescriptorSets(std::vector<VkWriteDescriptorSet> &descriptorWrites);

    VkDescriptorSet getDescSet() {return m_descriptorSet;}
    int getUniformCount() {return m_uniforms;}
    int getImageCount() {return m_images;}
private:
    GDeviceVLK &m_device;
    VkDescriptorSet m_descriptorSet;
    GDescriptorPoolVLK *m_parentPool;
    int m_uniforms = 0;
    int m_images = 0;
};


#endif //AWEBWOWVIEWERCPP_GDESCRIPTORSET_H
