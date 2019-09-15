//
// Created by Deamon on 7/8/2018.
//

#include "GM2ShaderPermutationVLK.h"
#include <string>
#include <fstream>




GM2ShaderPermutationVLK::GM2ShaderPermutationVLK(std::string &shaderName, IDevice *device, M2ShaderCacheRecord &permutation) :
                        GShaderPermutationVLK(shaderName, device) , permutation(permutation) {}

void GM2ShaderPermutationVLK::compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) {
    GShaderPermutationVLK::compileShader(vertExtraDef, fragExtraDef);

//    std::string fragmentExtraDefines = "";
//    std::string vertexExtraDefines = "";
//
//    vertexExtraDefines += "#define VERTEXSHADER " + std::to_string(permutation.vertexShader)+"\n";
//    vertexExtraDefines += "#define UNLIT " + std::to_string(permutation.unlit)+"\n";
//    vertexExtraDefines += "#define BONEINFLUENCES " + std::to_string(permutation.boneInfluences)+"\n";
//
//    fragmentExtraDefines += "#define FRAGMENTSHADER " + std::to_string(permutation.pixelShader)+"\n";
//    fragmentExtraDefines += "#define UNLIT " + std::to_string(permutation.unlit ? 1 : 0)+"\n";
//    fragmentExtraDefines += "#define ALPHATEST_ENABLED " + std::to_string(permutation.alphaTestOn ? 1 : 0)+"\n";
//    fragmentExtraDefines += "#define UNFOGGED " + std::to_string(permutation.unFogged ? 1 : 0)+"\n";
//    fragmentExtraDefines += "#define UNSHADOWED " + std::to_string(permutation.unShadowed ? 1 : 0)+"\n";
//
//
//
//    GM2ShaderPermutationVLK::compileShader(vertexExtraDefines, fragmentExtraDefines);


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

    for (int i = 0; i < 4; i++) {
        VkDescriptorSetLayoutBinding imageLayoutBinding = {};
        imageLayoutBinding.binding = 5+i;
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

    if (vkCreateDescriptorSetLayout(m_device->getVkDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

}
