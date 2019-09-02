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
}

