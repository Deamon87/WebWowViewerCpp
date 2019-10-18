//
// Created by Deamon on 7/1/2018.
//

#include <iostream>
#include "GShaderPermutationVLK.h"
#include "../../../engine/stringTrim.h"
#include "../../../engine/algorithms/hashString.h"
#include "../../../engine/shader/ShaderDefinitions.h"
#include "../../UniformBufferStructures.h"
#include "../../interface/IDevice.h"

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

void GShaderPermutationVLK::compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) {
    auto vertShaderCode = readFile("spirv/"+m_shaderName+".vert.spv");
    auto fragShaderCode = readFile("spirv/"+m_shaderName+".frag.spv");

    vertShaderModule = createShaderModule(vertShaderCode);
    fragShaderModule = createShaderModule(fragShaderCode);

    std::vector<VkDescriptorSetLayoutBinding> shaderLayoutBindings;
    for (int i = 0; i < 3; i++) {
        VkDescriptorSetLayoutBinding uboLayoutBinding = {};
        uboLayoutBinding.binding = i;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        shaderLayoutBindings.push_back(uboLayoutBinding);
    }
    for (int i = 3; i < 5; i++) {
        VkDescriptorSetLayoutBinding uboLayoutBinding = {};
        uboLayoutBinding.binding = i;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        shaderLayoutBindings.push_back(uboLayoutBinding);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = shaderLayoutBindings.size();
    layoutInfo.pBindings = &shaderLayoutBindings[0];

    if (vkCreateDescriptorSetLayout(m_device->getVkDevice(), &layoutInfo, nullptr, &uboDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    shaderLayoutBindings.clear();


    for (int i = 0; i < getTextureCount(); i++) {
        VkDescriptorSetLayoutBinding imageLayoutBinding = {};
        imageLayoutBinding.binding = getTextureBindingStart()+i;
        imageLayoutBinding.descriptorCount = 1;
        imageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imageLayoutBinding.pImmutableSamplers = nullptr;
        imageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        shaderLayoutBindings.push_back(imageLayoutBinding);
    }

    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = shaderLayoutBindings.size();
    layoutInfo.pBindings = &shaderLayoutBindings[0];

    if (vkCreateDescriptorSetLayout(m_device->getVkDevice(), &layoutInfo, nullptr, &uboDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

