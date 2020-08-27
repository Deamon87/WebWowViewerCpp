//
// Created by Deamon on 10/2/2019.
//

#include "GDescriptorSet.h"

GDescriptorSets::GDescriptorSets(IDevice &device, VkDescriptorSet descriptorSet,
    GDescriptorPoolVLK* parentPool, int uniforms, int images)
    : m_device(dynamic_cast<GDeviceVLK &>(device)), m_descriptorSet(descriptorSet), m_parentPool(parentPool),
    m_uniforms(uniforms), m_images(images) {

}
GDescriptorSets::~GDescriptorSets() {
    m_parentPool->deallocate(this);
};

void GDescriptorSets::writeToDescriptorSets(std::vector<VkWriteDescriptorSet> &descriptorWrites) {
    vkUpdateDescriptorSets(m_device.getVkDevice(), static_cast<uint32_t>(descriptorWrites.size()), &descriptorWrites[0], 0, nullptr);
}
