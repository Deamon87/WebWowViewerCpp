//
// Created by Deamon on 10/2/2019.
//

#include "GDescriptorSet.h"

#include "../textures/GTextureVLK.h"

GDescriptorSet::GDescriptorSet(const std::shared_ptr<IDeviceVulkan> &device, const std::shared_ptr<GDescriptorSetLayout> &hDescriptorSetLayout)
    : m_device(device), m_hDescriptorSetLayout(hDescriptorSetLayout) {

    m_descriptorSet = m_device->allocateDescriptorSetPrimitive(m_hDescriptorSetLayout, m_parentPool);
}
GDescriptorSet::~GDescriptorSet() {
    m_parentPool->deallocate(m_hDescriptorSetLayout, getDescSet());
};

GDescriptorSet::SetUpdateHelper GDescriptorSet::beginUpdate() {
    if (!m_firstUpdate) {
        //In this implementation of descriptor set, the descriptor set is getting free'd on update
        //and new one is created
        auto l_descriptorSet = m_descriptorSet;
        auto l_parentPool = m_parentPool;
        auto l_hDescriptorSetLayout = m_hDescriptorSetLayout;
        m_device->addDeallocationRecord([l_parentPool, l_descriptorSet, l_hDescriptorSetLayout]() {
            l_parentPool->deallocate(l_hDescriptorSetLayout, l_descriptorSet);
        });

        m_descriptorSet = m_device->allocateDescriptorSetPrimitive(m_hDescriptorSetLayout, m_parentPool);
    }
    return SetUpdateHelper(*this, boundDescriptors);
}

void GDescriptorSet::update() {
    auto update = beginUpdate();
}

// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkUpdateDescriptorSets.html :
// The operations described by pDescriptorWrites are performed first, followed by the operations described by pDescriptorCopies.
//
// So, writes first, copies second. T_T
void GDescriptorSet::writeToDescriptorSets(std::vector<VkWriteDescriptorSet> &descriptorWrites) {
    vkUpdateDescriptorSets(m_device->getVkDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

    m_firstUpdate = false;

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
    auto &slb = m_set.m_hDescriptorSetLayout->getShaderLayoutBindings();

    if (slb.find(bindIndex) == slb.end() || slb.at(bindIndex).descriptorType != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        std::cerr << "descriptor mismatch for image" << std::endl;
        throw std::runtime_error("descriptor mismatch for image");
    }

    if (textureVlk != nullptr) {
        assignBoundDescriptors(bindIndex, textureVlk, DescriptorRecord::DescriptorRecordType::Texture);
    }
    m_updateBindPoints[bindIndex] = true;

    VkDescriptorImageInfo &imageInfo = imageInfos.emplace_back();
    imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    if (textureVlk == nullptr || !textureVlk->getIsLoaded()) {
        auto blackTexture = std::dynamic_pointer_cast<GTextureVLK>(m_set.m_device->getBlackTexturePixel());
        imageInfo.imageView = blackTexture->texture.view;
        imageInfo.sampler = blackTexture->texture.sampler;
    } else {
        imageInfo.imageView = textureVlk->texture.view;
        imageInfo.sampler = textureVlk->texture.sampler;
    }

    VkWriteDescriptorSet &writeDescriptor = updates.emplace_back();
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
    auto &slb = m_set.m_hDescriptorSetLayout->getShaderLayoutBindings();

    if (slb.find(bindIndex) == slb.end() || slb.at(bindIndex).descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
        std::cerr << "descriptor mismatch for UBO dynamic" << std::endl;
        throw std::runtime_error("descriptor mismatch for UBO dynamic");
    }

    assignBoundDescriptors(bindIndex, buffer, DescriptorRecord::DescriptorRecordType::UBODynamic);
    m_updateBindPoints[bindIndex] = true;

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
    auto &slb = m_set.m_hDescriptorSetLayout->getShaderLayoutBindings();
    auto &uboSizes = m_set.m_hDescriptorSetLayout->getRequiredUBOSize();

    if (slb.find(bindIndex) == slb.end() || slb.at(bindIndex).descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
        std::cerr << "descriptor mismatch for UBO" << std::endl;
        throw std::runtime_error("descriptor mismatch for UBO");
    }
    if (uboSizes.find(bindIndex) != uboSizes.end() && buffer->getSize() != uboSizes.at(bindIndex)) {
        std::cout << "buffers missmatch! for"
                  << " binding = " << bindIndex
                  << " expected size " << uboSizes.at(bindIndex)
                  << ", provided size = " << (buffer->getSize())
                  << std::endl;
    }

    assignBoundDescriptors(bindIndex, buffer, DescriptorRecord::DescriptorRecordType::UBO);
    m_updateBindPoints[bindIndex] = true;

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

GDescriptorSet::SetUpdateHelper::~SetUpdateHelper() {
    //FOR DEBUG AND STABILITY

    //Fill the rest of Descriptor with already bound values
    //This is needed, cause In this implementation of descriptor set, the descriptor set is getting free'd on update
    //and new one is created
    for (int bindPoint = 0; bindPoint < m_updateBindPoints.size(); bindPoint++) {
        if(m_updateBindPoints[bindPoint] || m_boundDescriptors[bindPoint] == nullptr) continue;

        if (m_boundDescriptors[bindPoint]->descType == DescriptorRecord::DescriptorRecordType::UBO) {
            ubo(bindPoint, m_boundDescriptors[bindPoint]->buffer);
        } else if (m_boundDescriptors[bindPoint]->descType == DescriptorRecord::DescriptorRecordType::UBODynamic) {
            ubo_dynamic(bindPoint, m_boundDescriptors[bindPoint]->buffer);
        } else if (m_boundDescriptors[bindPoint]->descType == DescriptorRecord::DescriptorRecordType::Texture) {
            texture(bindPoint, m_boundDescriptors[bindPoint]->textureVlk);
        }
    }

    auto noSetBitSet =
        m_set.getDescSetLayout()->getRequiredBindPoints() & ~m_updateBindPoints;

    if (!noSetBitSet.none()) {
        std::string notSetBits;
        for (int i = 0; i < noSetBitSet.size(); i++) {
            notSetBits += " " + std::to_string(i);
        }

        std::cerr << "required descriptors " << notSetBits << " were not set during update" << std::endl;
        throw std::runtime_error("required descriptors were not set");
    }


    m_set.writeToDescriptorSets(updates);


}

