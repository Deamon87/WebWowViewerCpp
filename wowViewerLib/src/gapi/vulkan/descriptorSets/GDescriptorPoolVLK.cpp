//
// Created by Deamon on 10/2/2019.
//


#include <array>
#include "GDescriptorPoolVLK.h"

GDescriptorPoolVLK::GDescriptorPoolVLK(IDeviceVulkan &device, bool isBindless) : m_device(device) {
    uniformsAvailable = 4*4096;
    dynUniformsAvailable = 4*1024;
    ssboAvailable = 4*1024;
    imageAvailable = 4096 * 4;
    setsAvailable = 4096 * 4;

    std::array<VkDescriptorPoolSize, 3> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(uniformsAvailable);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(imageAvailable);
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(dynUniformsAvailable);
    //poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    //poolSizes[3].descriptorCount = static_cast<uint32_t>(ssboAvailable);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = setsAvailable;
    poolInfo.pNext = nullptr;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT ;
    if (isBindless)
        poolInfo.flags |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

    auto result = vkCreateDescriptorPool(m_device.getVkDevice(), &poolInfo, nullptr, &m_descriptorPool);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

VkDescriptorSet GDescriptorPoolVLK::allocate(const std::shared_ptr<GDescriptorSetLayout> &hDescriptorSetLayout) {
    if (uniformsAvailable < hDescriptorSetLayout->getTotalUbos() ||
        dynUniformsAvailable <  hDescriptorSetLayout->getTotalDynUbos() ||
        imageAvailable <  hDescriptorSetLayout->getTotalImages() ||
        setsAvailable < 1) return nullptr;

    constexpr int descSetCount = 1;
    std::array<VkDescriptorSetLayout, descSetCount> descLayouts = {hDescriptorSetLayout->getSetLayout()};

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = NULL;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = descSetCount;
    //VUID-VkDescriptorSetAllocateInfo-pSetLayouts-parameter
    //pSetLayouts must be a valid pointer to an array of __descriptorSetCount__ valid VkDescriptorSetLayout handles
    allocInfo.pSetLayouts = descLayouts.data();

    VkDescriptorSetVariableDescriptorCountAllocateInfoEXT count_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT};
    auto const &descSizes = hDescriptorSetLayout->getBindlessDescSizes();
    bool isBindless = !descSizes.empty();
    if (isBindless) {
        count_info.descriptorSetCount = descSizes.size();
        count_info.pDescriptorCounts  = descSizes.data();
        allocInfo.pNext = &count_info;
    }

    VkDescriptorSet descriptorSet;

    auto result = vkAllocateDescriptorSets(m_device.getVkDevice(), &allocInfo, &descriptorSet);
    if (result != VK_SUCCESS) {
        std::cerr << "Counting of descriptor sets went wrong" << std::endl;
        throw std::runtime_error("failed to allocate descriptor sets!");
//        return 0;
    }

    if (descriptorSet != VK_NULL_HANDLE) {
        uniformsAvailable -= hDescriptorSetLayout->getTotalUbos();
        dynUniformsAvailable -= hDescriptorSetLayout->getTotalDynUbos();
        imageAvailable -= hDescriptorSetLayout->getTotalImages();
        setsAvailable -= 1;
    }

    return descriptorSet;
}

void GDescriptorPoolVLK::deallocate(const std::shared_ptr<GDescriptorSetLayout> &hDescriptorLayout, VkDescriptorSet descSet) {
    auto h_this = this->shared_from_this();

    m_device.addDeallocationRecord([h_this, hDescriptorLayout, descSet]() {
        vkFreeDescriptorSets(h_this->m_device.getVkDevice(), h_this->m_descriptorPool, 1, &descSet);

        h_this->imageAvailable+= hDescriptorLayout->getTotalImages();
        h_this->uniformsAvailable+= hDescriptorLayout->getTotalUbos();
        h_this->dynUniformsAvailable+= hDescriptorLayout->getTotalDynUbos();
        h_this->setsAvailable+=1;
    });
}
