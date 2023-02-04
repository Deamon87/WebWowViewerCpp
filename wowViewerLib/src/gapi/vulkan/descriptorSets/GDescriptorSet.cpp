//
// Created by Deamon on 10/2/2019.
//

#include "GDescriptorSet.h"

#include "../textures/GTextureVLK.h"

GDescriptorSet::GDescriptorSet(IDevice &device,
                               std::shared_ptr<GDescriptorSetLayout> &hDescriptorSetLayout,
                               VkDescriptorSet descriptorSet,
                               std::shared_ptr<GDescriptorPoolVLK> parentPool)
    : m_device(dynamic_cast<GDeviceVLK &>(device)),
      m_parentPool(parentPool),
      m_descriptorSet(descriptorSet),
      m_hDescriptorSetLayout(hDescriptorSetLayout) {

}
GDescriptorSet::~GDescriptorSet() {
    m_parentPool->deallocate(this);
};

void GDescriptorSet::update() {
    auto update = beginUpdate();
}

// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkUpdateDescriptorSets.html :
// The operations described by pDescriptorWrites are performed first, followed by the operations described by pDescriptorCopies.
//
// So, writes first, copies second. T_T
void GDescriptorSet::writeToDescriptorSets(std::vector<VkWriteDescriptorSet> &descriptorWrites) {
    vkUpdateDescriptorSets(m_device.getVkDevice(), static_cast<uint32_t>(descriptorWrites.size()), &descriptorWrites[0], 0, nullptr);

//    m_updateBitSet |= updatedBindsBitSet;
//
//    for (int i = 0; updatedBindsBitSet.size(); i++) {
//        m_updatesLeft[i] = GDeviceVLK::MAX_FRAMES_IN_FLIGHT;
//    }
}


// -------------------------------------
// Update Helper
// -------------------------------------

GDescriptorSet::SetUpdateHelper &
GDescriptorSet::SetUpdateHelper::texture(int bindIndex, const std::shared_ptr<GTextureVLK> &textureVlk) {
    VkDescriptorImageInfo &imageInfo = imageInfos.emplace_back();
    imageInfo = {};

    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = textureVlk->texture.view;
    imageInfo.sampler = textureVlk->texture.sampler;

    VkWriteDescriptorSet writeDescriptor = updates.emplace_back();
    writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptor.dstSet = m_set.getDescSet();
    writeDescriptor.pNext = nullptr;
    writeDescriptor.dstBinding = bindIndex;
    writeDescriptor.dstArrayElement = 0;
    writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptor.descriptorCount = 1;
    writeDescriptor.pBufferInfo = nullptr;
    writeDescriptor.pImageInfo = &imageInfo;
    writeDescriptor.pTexelBufferView = nullptr;

    return *this;
}

//NOTE: Dynamic UBO still requires the buffer to be bound, but the offset can be served in runtime.
//And this current system of sub-allocation it seems, there is no real reason to rely on this. Sort of.
GDescriptorSet::SetUpdateHelper &
GDescriptorSet::SetUpdateHelper::ubo_dynamic(int bindIndex, const std::shared_ptr<IBufferVLK> &buffer) {
    VkDescriptorBufferInfo &bufferInfo = bufferInfos.emplace_back();
    bufferInfo = {};
    bufferInfo.buffer = buffer->getGPUBuffer();
    bufferInfo.offset = buffer->getOffset();
    bufferInfo.range = buffer->getSize();


    VkWriteDescriptorSet &writeDescriptor = updates.emplace_back();
    writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptor.dstSet = m_set.getDescSet();
    writeDescriptor.pNext = nullptr;
    writeDescriptor.dstBinding = bindIndex;
    writeDescriptor.dstArrayElement = 0;
    writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    writeDescriptor.descriptorCount = 1;
    writeDescriptor.pBufferInfo = &bufferInfo;
    writeDescriptor.pImageInfo = nullptr;
    writeDescriptor.pTexelBufferView = nullptr;

    return *this;
}

GDescriptorSet::SetUpdateHelper &
GDescriptorSet::SetUpdateHelper::ubo(int bindIndex, const std::shared_ptr<IBufferVLK> &buffer) {
    VkDescriptorBufferInfo &bufferInfo = bufferInfos.emplace_back();
    bufferInfo = {};
    bufferInfo.buffer = buffer->getGPUBuffer();
    bufferInfo.offset = buffer->getOffset();
    bufferInfo.range = buffer->getSize();


    VkWriteDescriptorSet &writeDescriptor = updates.emplace_back();
    writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptor.dstSet = m_set.getDescSet();
    writeDescriptor.pNext = nullptr;
    writeDescriptor.dstBinding = bindIndex;
    writeDescriptor.dstArrayElement = 0;
    writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptor.descriptorCount = 1;
    writeDescriptor.pBufferInfo = &bufferInfo;
    writeDescriptor.pImageInfo = nullptr;
    writeDescriptor.pTexelBufferView = nullptr;

    return *this;
}

GDescriptorSet::SetUpdateHelper &
GDescriptorSet::SetUpdateHelper::ssbo(int bindIndex, const std::shared_ptr<IBufferVLK> &buffer) {
    throw "unimplemented";

    return *this;
}

