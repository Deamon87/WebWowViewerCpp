//
// Created by Deamon on 7/1/2018.
//

#include <iostream>
#include "GShaderPermutationVLK.h"
#include "../../../engine/stringTrim.h"
#include "../../../engine/algorithms/hashString.h"
#include "../../../engine/shader/ShaderDefinitions.h"
#include "../../UniformBufferStructures.h"
#include "../buffers/GUniformBufferVLK.h"
#include "../../interface/IDevice.h"
#include <unordered_map>

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkShaderModule GShaderPermutationVLK::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_device->getVkDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

GShaderPermutationVLK::GShaderPermutationVLK(std::string &shaderName, IDevice * device) :
    m_device(dynamic_cast<GDeviceVLK *>(device)), m_shaderName(shaderName){

}

void GShaderPermutationVLK::createUBODescriptorLayout() {
    std::unordered_map<int,VkDescriptorSetLayoutBinding> shaderLayoutBindings;
    for (int i = 0; i < vertShaderMeta->uboBindings.size(); i++) {
        auto &uboVertBinding = vertShaderMeta->uboBindings[i];


        if (auto it{ shaderLayoutBindings.find(uboVertBinding.binding) }; it != std::end( shaderLayoutBindings )) {
            it->second.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
        } else {
            VkDescriptorSetLayoutBinding uboLayoutBinding = {};
            uboLayoutBinding.binding = uboVertBinding.binding;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            shaderLayoutBindings.insert({uboVertBinding.binding, uboLayoutBinding});
        }

        hasBondUBO[uboVertBinding.binding] = true;

    }
    for (int i = 0; i < fragShaderMeta->uboBindings.size(); i++) {
        auto &uboFragBinding = fragShaderMeta->uboBindings[i];

        if (auto it{ shaderLayoutBindings.find(uboFragBinding.binding) }; it != std::end( shaderLayoutBindings )) {
            it->second.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        } else {
            VkDescriptorSetLayoutBinding uboLayoutBinding = {};
            uboLayoutBinding.binding = uboFragBinding.binding;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            shaderLayoutBindings.insert({uboFragBinding.binding, uboLayoutBinding});
        }

        hasBondUBO[uboFragBinding.binding] = true;
    }

    std::vector<VkDescriptorSetLayoutBinding> shaderLayoutBindingsVec;
    shaderLayoutBindingsVec.reserve(shaderLayoutBindings.size());
    for(auto elem : shaderLayoutBindings) {
        shaderLayoutBindingsVec.push_back(elem.second);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = NULL;
    layoutInfo.bindingCount = shaderLayoutBindingsVec.size();
    layoutInfo.pBindings = &shaderLayoutBindingsVec[0];

    if (vkCreateDescriptorSetLayout(m_device->getVkDevice(), &layoutInfo, nullptr, &uboDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void GShaderPermutationVLK::createImageDescriptorLayout() {
    std::vector<VkDescriptorSetLayoutBinding> shaderLayoutBindings;

    for (int i = 0; i < getTextureCount(); i++) {
        VkDescriptorSetLayoutBinding imageLayoutBinding = {};
        imageLayoutBinding.binding = getTextureBindingStart()+i;
        imageLayoutBinding.descriptorCount = 1;
        imageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imageLayoutBinding.pImmutableSamplers = nullptr;
        imageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        shaderLayoutBindings.push_back(imageLayoutBinding);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = shaderLayoutBindings.size();
    layoutInfo.pBindings = &shaderLayoutBindings[0];

    if (vkCreateDescriptorSetLayout(m_device->getVkDevice(), &layoutInfo, nullptr, &imageDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void GShaderPermutationVLK::updateDescriptorSet(int index) {
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    std::vector<VkWriteDescriptorSet> descriptorWrites;

    auto *uploadBuffer = ((GUniformBufferVLK *) m_device->getUploadBuffer(index).get());

    for (int i = 0; i < vertShaderMeta->uboBindings.size(); i++) {
        auto &uboVertBinding = vertShaderMeta->uboBindings[i];

        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uploadBuffer->g_buf;
        bufferInfo.offset = 0;
        bufferInfo.range = uboVertBinding.size;
        bufferInfos.push_back(bufferInfo);
    }
    for (int i = 0; i < fragShaderMeta->uboBindings.size(); i++) {
        auto &uboFragBinding = fragShaderMeta->uboBindings[i];

        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uploadBuffer->g_buf;
        bufferInfo.offset = 0;
        bufferInfo.range = uboFragBinding.size;
        bufferInfos.push_back(bufferInfo);
    }

    int buffInd = 0;
    for (int i = 0; i < vertShaderMeta->uboBindings.size(); i++) {
        auto &uboVertBinding = vertShaderMeta->uboBindings[i];

        VkWriteDescriptorSet writeDescriptor;
        writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptor.dstSet = uboDescriptorSets[index]->getDescSet();
        writeDescriptor.pNext = nullptr;
        writeDescriptor.dstBinding = uboVertBinding.binding;
        writeDescriptor.dstArrayElement = 0;
        writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        writeDescriptor.descriptorCount = 1;
        writeDescriptor.pBufferInfo = &bufferInfos[buffInd++];
        writeDescriptor.pImageInfo = nullptr;
        writeDescriptor.pTexelBufferView = nullptr;
        descriptorWrites.push_back(writeDescriptor);

    }
    for (int i = 0; i < fragShaderMeta->uboBindings.size(); i++) {
        auto &uboFragBinding = fragShaderMeta->uboBindings[i];

        VkWriteDescriptorSet writeDescriptor;
        writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptor.dstSet = uboDescriptorSets[index]->getDescSet();
        writeDescriptor.pNext = nullptr;
        writeDescriptor.dstBinding = uboFragBinding.binding;
        writeDescriptor.dstArrayElement = 0;
        writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        writeDescriptor.descriptorCount = 1;
        writeDescriptor.pBufferInfo = &bufferInfos[buffInd++];
        writeDescriptor.pImageInfo = nullptr;
        writeDescriptor.pTexelBufferView = nullptr;
        descriptorWrites.push_back(writeDescriptor);
    }

    uboDescriptorSets[index]->writeToDescriptorSets(descriptorWrites);
}

void GShaderPermutationVLK::createUboDescriptorSets() {
    uboDescriptorSets = std::vector<std::shared_ptr<GDescriptorSets>>(4, NULL);

    for (int j = 0; j < 4; j++) {
        std::vector<VkWriteDescriptorSet> descriptorWrites;
        uboDescriptorSets[j] = m_device->createDescriptorSet(uboDescriptorSetLayout, 5, 0);
        updateDescriptorSet(j);
    }
}

void GShaderPermutationVLK::compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) {
    auto vertShaderCode = readFile("spirv/" + m_shaderName + ".vert.spv");
    auto fragShaderCode = readFile("spirv/" + m_shaderName + ".frag.spv");

    vertShaderModule = createShaderModule(vertShaderCode);
    fragShaderModule = createShaderModule(fragShaderCode);

    vertShaderMeta = &shaderMetaInfo.at(m_shaderName + ".vert.spv");
    fragShaderMeta = &shaderMetaInfo.at(m_shaderName + ".frag.spv");


    this->createUBODescriptorLayout();
    this->createImageDescriptorLayout();
    this->createUboDescriptorSets();
}

