//
// Created by Deamon on 29.12.22.
//

#include "IMaterialVLK.h"
#include "../shaders/GShaderPermutationVLK.h"
#include "../textures/GTextureVLK.h"

void IMaterialVLK::createImageDescriptorSet() {
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

void IMaterialVLK::updateImageDescriptorSet() {
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

IMaterialVLK::IMaterialVLK(HGDeviceVLK device) : m_device(device) {

    //TODO:
    m_shader =  device->getShader("", "", nullptr);

    auto shaderVLK = std::dynamic_pointer_cast<GShaderPermutationVLK>(m_shader);


    //Check the buffer sizes
    std::unordered_map<int,uboBindingData> shaderLayoutBindings;
    for (int i = 0; i < shaderVLK->vertShaderMeta->uboBindings.size(); i++) {
        auto &uboVertBinding = shaderVLK->vertShaderMeta->uboBindings[i];

        auto it = shaderLayoutBindings.find(uboVertBinding.binding);
        if (it == std::end( shaderLayoutBindings )) {
            shaderLayoutBindings.insert({uboVertBinding.binding, uboVertBinding});
        }
    }
    for (int i = 0; i < shaderVLK->fragShaderMeta->uboBindings.size(); i++) {
        auto &uboFragBinding = shaderVLK->fragShaderMeta->uboBindings[i];
        auto it = shaderLayoutBindings.find(uboFragBinding.binding);
        if (it == std::end( shaderLayoutBindings )) {
            shaderLayoutBindings.insert({uboFragBinding.binding, uboFragBinding});
        }
    }
//TODO:
//    for (int i = 0; i < 5; i++) {
//        auto it = shaderLayoutBindings.find(i);
//        if (it != shaderLayoutBindings.end()) {
//            if ((m_UniformBuffer[i] != nullptr) && (it->second.size != (m_UniformBuffer[i]->getSize()))) {
//                std::cout << "buffers missmatch! for shaderName = " << shaderVLK->getShaderName() <<
//                " index = " << i <<
//                " expected size " << (it->second.size) <<
//                ", provided size = " << (m_UniformBuffer[i]->getSize()) <<
//                std::endl;
//            }
//        }
//    }

}
