//
// Created by Deamon on 29.12.22.
//

#include "ISimpleMaterialVLK.h"
#include "../shaders/GShaderPermutationVLK.h"
#include "../textures/GTextureVLK.h"

void ISimpleMaterialVLK::createImageDescriptorSet() {
    auto shaderVLK = std::dynamic_pointer_cast<GShaderPermutationVLK>(m_shader);
    auto descLayout = shaderVLK->getImageDescriptorLayout();

    imageDescriptorSets = { m_device->createDescriptorSet(descLayout, 0, shaderVLK->getTextureCount()) };

    {
        std::vector<VkWriteDescriptorSet> descriptorWrites;

        //Bind Black pixel texture
        std::vector<VkDescriptorImageInfo> imageInfos(shaderVLK->getTextureCount());

        auto blackTexture = m_device->getBlackPixelTexture();
        auto blackTextureVlk = std::dynamic_pointer_cast<GTextureVLK>(blackTexture);

        int bindIndex = 0;
        for (int i = 0; i < shaderVLK->getTextureCount(); i++) {
            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = blackTextureVlk->texture.view;
            imageInfo.sampler = blackTextureVlk->texture.sampler;
            imageInfos[bindIndex] = imageInfo;

            VkWriteDescriptorSet writeDescriptor;
            writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptor.dstSet = imageDescriptorSets[0]->getDescSet();
            writeDescriptor.pNext = nullptr;
            writeDescriptor.dstBinding = shaderVLK->getTextureBindingStart()+bindIndex;
            writeDescriptor.dstArrayElement = 0;
            writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeDescriptor.descriptorCount = 1;
            writeDescriptor.pBufferInfo = nullptr;
            writeDescriptor.pImageInfo = &imageInfos[bindIndex];
            writeDescriptor.pTexelBufferView = nullptr;
            descriptorWrites.push_back(writeDescriptor);
            bindIndex++;
        }

        if (!descriptorWrites.empty()) {
            imageDescriptorSets[0]->writeToDescriptorSets(descriptorWrites);
        }
    }
}

void ISimpleMaterialVLK::updateImageDescriptorSet() {
    bool allTexturesAreReady = true;

    auto shaderVLK = std::dynamic_pointer_cast<GShaderPermutationVLK>(m_shader);
    int textureBegin = shaderVLK->getTextureBindingStart();

    for (auto& texture : m_textures) {
        if (texture == nullptr) continue;
        allTexturesAreReady &= texture->getIsLoaded();
    }

    if (allTexturesAreReady) {
        std::vector<VkWriteDescriptorSet> descriptorWrites;
        std::vector<VkDescriptorImageInfo> imageInfos(m_textures.size());

        if (shaderVLK->getTextureCount() == 0) return;

        for (size_t i = 0; i < m_textures.size(); i++) {
            auto textureVlk = std::dynamic_pointer_cast<GTextureVLK>(m_textures[i]);
            if (textureVlk == nullptr) continue;

            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textureVlk->texture.view;
            imageInfo.sampler = textureVlk->texture.sampler;
            imageInfos[i] = imageInfo;

            VkWriteDescriptorSet writeDescriptor;
            writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptor.dstSet = imageDescriptorSets[0]->getDescSet();
            writeDescriptor.pNext = nullptr;
            writeDescriptor.dstBinding = textureBegin+i;
            writeDescriptor.dstArrayElement = 0;
            writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeDescriptor.descriptorCount = 1;
            writeDescriptor.pImageInfo = &imageInfos[i];
            descriptorWrites.push_back(writeDescriptor);
        }

        if (descriptorWrites.size() > 0) {
            imageDescriptorSets[0]->writeToDescriptorSets(descriptorWrites);
//            vkUpdateDescriptorSets(m_device.getVkDevice(), static_cast<uint32_t>(descriptorWrites.size()), &descriptorWrites[0], 0, nullptr);
        }
    }
}

ISimpleMaterialVLK::ISimpleMaterialVLK(const HGDeviceVLK &device, std::string vertexShader, std::string pixelShader, std::vector<std::shared_ptr<IBufferVLK>> &ubos, std::vector<HGTextureVLK> &textures) : m_device(device) {
    m_shader =  device->getShader(vertexShader, pixelShader, nullptr);

    auto shaderVLK = std::dynamic_pointer_cast<GShaderPermutationVLK>(m_shader);
    auto &shaderLayout = shaderVLK->getShaderLayout();

    auto uboSetLayout = shaderLayout.setLayouts[0];
    if (ubos.size() != uboSetLayout.uboBindings.length) {
        std::cerr << "not enough ubos for shaderName = " << shaderVLK->getShaderName() << std::endl;
    }
    for (unsigned int i = uboSetLayout.uboBindings.start; i <= uboSetLayout.uboBindings.end; i++) {
        auto it = uboSetLayout.uboSizesPerBinding.find(i);
        if (it != std::end(uboSetLayout.uboSizesPerBinding)) {
            auto uboIndex = i - uboSetLayout.uboBindings.start;
            if (ubos[uboIndex] == nullptr) {
                std::cerr << "UBO is not set for "
                          << "shader = " << shaderVLK->getShaderName()
                          << " set = " << 1
                          << " binding" << i
                          << std::endl;
            }

            if (it->second != ubos[uboIndex]->getSize()) {
                std::cout << "buffers missmatch! for"
                          << " shaderName = " << shaderVLK->getShaderName()
                          << " set = " << 1
                          << " binding = " << i
                          << " expected size " << (it->second)
                          << ", provided size = " << (ubos[uboIndex]->getSize())
                          << std::endl;
            }
        }
    }

    auto imageSetLayout = shaderLayout.setLayouts[1];
    if (imageSetLayout.imageBindings.length != textures.size()) {
        std::cout << "image count mismatch! for"
                  << " shaderName = " << shaderVLK->getShaderName()
                  << " set = " << 1
                  << " expected count " << uboSetLayout.imageBindings.length
                  << ", provided count = " << textures.size()
                  << std::endl;
    }

    m_textures = textures;
    m_ubos = ubos;
}
