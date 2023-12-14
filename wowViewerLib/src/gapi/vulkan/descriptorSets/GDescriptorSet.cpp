//
// Created by Deamon on 10/2/2019.
//

#include "GDescriptorSet.h"

#include "../textures/GTextureVLK.h"
#include "../textures/GTextureSamplerVLK.h"
#include "../GDescriptorSetUpdater.h"

GDescriptorSet::GDescriptorSet(const std::shared_ptr<IDeviceVulkan> &device, const std::shared_ptr<GDescriptorSetLayout> &hDescriptorSetLayout)
    : m_device(device), m_hDescriptorSetLayout(hDescriptorSetLayout) {

    //Gets DS and gets pool it was allocated from into m_parentPool
    m_descriptorSet = m_device->allocateDescriptorSetPrimitive(m_hDescriptorSetLayout, m_parentPool);

    auto &arraySizes = hDescriptorSetLayout->getArraySizes();
    for (int i = 0; i < arraySizes.size(); i++) {
        boundDescriptors[i].resize(arraySizes[i]);
    };

    if (m_hDescriptorSetLayout->getIsBindless()) {
        m_bindlessAccum = std::make_unique<BindlessUpdateAccum>();
    }

    assert(m_descriptorSet != nullptr);
}
GDescriptorSet::~GDescriptorSet() {
    m_parentPool->deallocate(m_hDescriptorSetLayout, getDescSet());
};

GDescriptorSet::SetUpdateHelper GDescriptorSet::beginUpdate() {
    if (!m_hDescriptorSetLayout->getIsBindless()) {
        if (!m_firstUpdate) {
            //For normal (non-bindless) implementation of descriptor set, the descriptor set is getting free'd on update
            //and new one is created

            //This deallocate already have deallocate queue queue submission inside
            m_parentPool->deallocate(m_hDescriptorSetLayout, m_descriptorSet);

            m_descriptorSet = m_device->allocateDescriptorSetPrimitive(m_hDescriptorSetLayout, m_parentPool);
            assert(m_descriptorSet != nullptr);
        }
    }
    return SetUpdateHelper(*this, boundDescriptors, m_device->getDescriptorSetUpdater());
}

void GDescriptorSet::update() {
    auto update = beginUpdate();
}

// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkUpdateDescriptorSets.html :
// The operations described by pDescriptorWrites are performed first, followed by the operations described by pDescriptorCopies.
//
// So, writes first, copies second. T_T
void GDescriptorSet::writeToDescriptorSets(framebased::vector<VkWriteDescriptorSet> &descriptorWrites, framebased::vector<VkDescriptorImageInfo> &imageInfo, framebased::vector<int> &dynamicBufferIndexes) {
    vkUpdateDescriptorSets(m_device->getVkDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

    m_firstUpdate = false;

    m_dynamicBufferIndexes = decltype(m_dynamicBufferIndexes)(dynamicBufferIndexes.begin(), dynamicBufferIndexes.end());
    m_dynamicBuffers.resize(m_dynamicBufferIndexes.size());
    for (int i = 0; i < m_dynamicBufferIndexes.size(); i++) {
        m_dynamicBuffers[i] = boundDescriptors[m_dynamicBufferIndexes[i]][0]->buffer.get();
    }
}



// -------------------------------------
// Update Helper
// -------------------------------------

GDescriptorSet::SetUpdateHelper &
GDescriptorSet::SetUpdateHelper::texture(int bindIndex, const HGSamplableTexture &samplableTextureVlk, int index) {
    auto &slb = m_set.m_hDescriptorSetLayout->getShaderLayoutBindings();

#if (!defined(NDEBUG))
    if (slb.find(bindIndex) == slb.end() || slb.at(bindIndex).descriptorType != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        std::cerr << "descriptor mismatch for image" << std::endl;
        throw std::runtime_error("descriptor mismatch for image");
    }
#endif

    auto textureVlk = samplableTextureVlk != nullptr ? ((GTextureVLK *)samplableTextureVlk->getTexture().get()) : nullptr;
    auto samplerVlk = samplableTextureVlk != nullptr ? ((GTextureSamplerVLK *)samplableTextureVlk->getSampler().get()) : nullptr;

    assignBoundDescriptors(bindIndex, samplableTextureVlk, index, DescriptorRecord::DescriptorRecordType::Texture);

    auto texture = textureVlk;
    VkDescriptorImageInfo &imageInfo = imageInfos.emplace_back();

    imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    if (textureVlk == nullptr || !textureVlk->getIsLoaded()) {
        auto blackTextureVlk = m_set.m_device->getBlackTexturePixel();

        texture = (GTextureVLK *) blackTextureVlk->getTexture().get();
        samplerVlk = (GTextureSamplerVLK *) blackTextureVlk->getSampler().get();
    }

    imageInfo.imageView = texture->texture.view;
    imageInfo.sampler = samplerVlk->getSampler();

    VkWriteDescriptorSet &writeDescriptor = updates.emplace_back();
    writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptor.dstSet = m_set.getDescSet();
    writeDescriptor.pNext = nullptr;
    writeDescriptor.dstBinding = bindIndex;
    writeDescriptor.dstArrayElement = index;
    writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptor.descriptorCount = 1;
    writeDescriptor.pBufferInfo = nullptr;
    writeDescriptor.pImageInfo = &imageInfo;
    writeDescriptor.pTexelBufferView = nullptr;

    assert(writeDescriptor.pImageInfo == &imageInfos[imageInfos.size()-1]);

    m_updateBindPoints[bindIndex] = true;

    return *this;
}

//NOTE: Dynamic UBO still requires the buffer to be bound, but the offset can be served in runtime.
//And this current system of sub-allocation it seems, there is no real reason to rely on this. Sort of.
GDescriptorSet::SetUpdateHelper &
GDescriptorSet::SetUpdateHelper::ubo_dynamic(int bindIndex, const std::shared_ptr<IBufferVLK> &buffer) {
    auto &slb = m_set.m_hDescriptorSetLayout->getShaderLayoutBindings();
    typeOverrides[bindIndex] = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

#if (!defined(NDEBUG))
    if (slb.find(bindIndex) == slb.end() || slb.at(bindIndex).descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
        std::cerr << "descriptor mismatch for UBO dynamic" << std::endl;
        throw std::runtime_error("descriptor mismatch for UBO dynamic");
    }
#endif

    assignBoundDescriptors(bindIndex, buffer, 0, DescriptorRecord::DescriptorRecordType::UBODynamic);
    m_updateBindPoints[bindIndex] = true;

    VkDescriptorBufferInfo &bufferInfo = bufferInfos.emplace_back();
    bufferInfo = {};
    bufferInfo.buffer = buffer->getGPUBuffer();
    bufferInfo.offset = 0; //Mandatory for VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC to work
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

    dynamicBufferIndexes.push_back(bindIndex);

    return *this;
}

GDescriptorSet::SetUpdateHelper &
GDescriptorSet::SetUpdateHelper::ubo(int bindIndex, const std::shared_ptr<IBufferVLK> &buffer) {
    auto &slb = m_set.m_hDescriptorSetLayout->getShaderLayoutBindings();
    auto &uboSizes = m_set.m_hDescriptorSetLayout->getRequiredUBOSize();

#if (!defined(NDEBUG))
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
#endif


    assignBoundDescriptors(bindIndex, buffer, 0, DescriptorRecord::DescriptorRecordType::UBO);
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
    auto &slb = m_set.m_hDescriptorSetLayout->getShaderLayoutBindings();
    auto &ssboSizes = m_set.m_hDescriptorSetLayout->getRequiredSSBOSize();

#if (!defined(NDEBUG))
    if (slb.find(bindIndex) == slb.end() || slb.at(bindIndex).descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
        std::cerr << "descriptor mismatch for SSBO" << std::endl;
        throw std::runtime_error("descriptor mismatch for UBO");
    }
    if (ssboSizes.find(bindIndex) != ssboSizes.end() && buffer->getSize() != ssboSizes.at(bindIndex)) {
        std::cout << "buffers missmatch! for"
                  << " binding = " << bindIndex
                  << " expected size " << ssboSizes.at(bindIndex)
                  << ", provided size = " << (buffer->getSize())
                  << std::endl;
    }
#endif


    assignBoundDescriptors(bindIndex, buffer, 0, DescriptorRecord::DescriptorRecordType::SSBO);
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
    writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptor.descriptorCount = 1;
    writeDescriptor.pBufferInfo = &bufferInfo;
    writeDescriptor.pImageInfo = nullptr;
    writeDescriptor.pTexelBufferView = nullptr;

    return *this;
}

GDescriptorSet::SetUpdateHelper::~SetUpdateHelper() {
    if (updateCancelled) return;

#if (!defined(NDEBUG))
    {
        int bufferIndex = 0;
        int imageIndex = 0;
        for (int i = 0; i < updates.size(); i++) {
            if (updates[i].pBufferInfo != nullptr) {
                assert(updates[i].pBufferInfo == &bufferInfos[bufferIndex++]);
            } else if (updates[i].pImageInfo != nullptr) {
                assert(updates[i].pImageInfo == &imageInfos[imageIndex++]);
            }
        }
    }
#endif

    //FOR DEBUG AND STABILITY
    if (!m_set.m_hDescriptorSetLayout->getIsBindless()) {
        //Fill the rest of Descriptor with already bound values
        //This is needed, cause In this implementation of descriptor set, the descriptor set is getting free'd on update
        //and new one is created
        for (int bindPoint = 0; bindPoint < m_updateBindPoints.size(); bindPoint++) {
            if (m_updateBindPoints[bindPoint]) continue;

            for (int bindIndex = 0; bindIndex < m_boundDescriptors[bindPoint].size(); bindIndex++) {
                if (m_boundDescriptors[bindPoint][bindIndex] == nullptr) continue;

                reassignBinding(bindPoint, bindIndex);
            }
        }

        auto noSetBitSet =
            m_set.getDescSetLayout()->getRequiredBindPoints() & (~m_updateBindPoints);

        if (!noSetBitSet.none()) {
            std::string notSetBits;
            for (int i = 0; i < noSetBitSet.size(); i++) {
                if (noSetBitSet[i]) {
                    notSetBits += " " + std::to_string(i);
                }
            }

            std::cerr << "required descriptors " << notSetBits << " were not set during update" << std::endl;
            throw std::runtime_error("required descriptors were not set");
        }
    } else {
        auto updates = m_set.m_bindlessAccum->getUpdates();
        for (auto const &update: updates.get()){
            reassignBinding(update.bindIndex, update.arrayIndex);
        }
    }

    std::sort(dynamicBufferIndexes.begin(), dynamicBufferIndexes.end());
    dynamicBufferIndexes.erase(std::unique(dynamicBufferIndexes.begin(), dynamicBufferIndexes.end()), dynamicBufferIndexes.end());

    m_set.writeToDescriptorSets(updates, imageInfos, dynamicBufferIndexes);

#if (!defined(NDEBUG))
    {
        int bufferIndex = 0;
        int imageIndex = 0;
        for (int i = 0; i < updates.size(); i++) {
            if (updates[i].pBufferInfo != nullptr) {
                assert(updates[i].pBufferInfo == &bufferInfos[bufferIndex++]);
            } else if (updates[i].pImageInfo != nullptr) {
                assert(updates[i].pImageInfo == &imageInfos[imageIndex++]);
            }
        }
    }
#endif
}

void GDescriptorSet::SetUpdateHelper::reassignBinding(int bindPoint, int bindIndex) {
    if (m_boundDescriptors[bindPoint][bindIndex]->descType == DescriptorRecord::DescriptorRecordType::UBO) {
        ubo(bindPoint, m_boundDescriptors[bindPoint][bindIndex]->buffer);
    } else if (m_boundDescriptors[bindPoint][bindIndex]->descType ==
               DescriptorRecord::DescriptorRecordType::UBODynamic) {
        ubo_dynamic(bindPoint, m_boundDescriptors[bindPoint][bindIndex]->buffer);
    } else if (m_boundDescriptors[bindPoint][bindIndex]->descType == DescriptorRecord::DescriptorRecordType::SSBO) {
        ssbo(bindPoint, m_boundDescriptors[bindPoint][bindIndex]->buffer);
    } else if (m_boundDescriptors[bindPoint][bindIndex]->descType == DescriptorRecord::DescriptorRecordType::Texture) {
        texture(bindPoint, m_boundDescriptors[bindPoint][bindIndex]->texture, bindIndex);
    }
}

void GDescriptorSet::SetUpdateHelper::delayUpdate() {
    updateCancelled = true;

    m_descriptorSetUpdater->addToUpdate(m_set.shared_from_this());
}

std::function<void()> GDescriptorSet::SetUpdateHelper::createCallback(int bindPoint, int arrayIndex) {
    auto ds_weak = m_set.weak_from_this();
    auto l_descriptorSetUpdater = m_descriptorSetUpdater;
    if (!m_set.m_hDescriptorSetLayout->getIsBindless()) {
        const std::function<void()> callback = ([ds_weak, l_descriptorSetUpdater]() -> void {
            if (auto ds = ds_weak.lock()) {
                l_descriptorSetUpdater->addToUpdate(ds);
            }
        });

        return callback;
    } else {
        const std::function<void()> callback = ([ds_weak, l_descriptorSetUpdater, bindPoint, arrayIndex]() -> void {
            if (auto ds = ds_weak.lock()) {
                ds->m_bindlessAccum->addUpdate(bindPoint, arrayIndex);
                l_descriptorSetUpdater->addToUpdate(ds);
            }
        });

        return callback;
    }
}

