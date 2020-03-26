//
// Created by Deamon on 10/2/2019.
//


#include <array>
#include "GDescriptorPoolVLK.h"
#include "../shaders/GShaderPermutationVLK.h"

GDescriptorPoolVLK::GDescriptorPoolVLK(IDevice &device) : m_device(dynamic_cast<GDeviceVLK &>(device)) {

    uniformsAvailable = 5*4096;
    imageAvailable = 4096 * 4;
    setsAvailable = 4096;

    std::array<VkDescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(uniformsAvailable);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(imageAvailable);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = setsAvailable;

    if (vkCreateDescriptorPool(m_device.getVkDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

std::shared_ptr<GDescriptorSets> GDescriptorPoolVLK::allocate(VkDescriptorSetLayout layout, int uniforms, int images) {
    if (uniformsAvailable < uniforms || imageAvailable < images || setsAvailable < 1) return nullptr;

    std::array<VkDescriptorSetLayout,4> descLAyouts = {layout,layout,layout,layout};
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = NULL;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = descLAyouts.data();


    VkDescriptorSet descriptorSet;

    if (vkAllocateDescriptorSets(m_device.getVkDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    uniformsAvailable -= uniforms;
    imageAvailable -= images;
	setsAvailable -= 1;

    std::shared_ptr<GDescriptorSets> result = std::make_shared<GDescriptorSets>(m_device, descriptorSet);
    return result;
}
