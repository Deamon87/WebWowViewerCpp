//
// Created by Deamon on 7/1/2018.
//

#include <iostream>
#include "GShaderPermutationVLK.h"
#include <ShaderDefinitions.h>

GShaderPermutationVLK::GShaderPermutationVLK(const std::string &shaderVertName, const std::string &shaderFragName,
                                             const std::shared_ptr<GDeviceVLK> &device,
                                             const ShaderConfig &shaderConf,
                                             const std::unordered_map<int, const std::shared_ptr<GDescriptorSetLayout>> &dsLayoutOverrides
                                             ) :
    GShaderPermutationVLKBase(shaderVertName + " " + shaderFragName, device, shaderConf.typeOverrides, dsLayoutOverrides),
    m_shaderConfig(shaderConf), m_shaderNameVert(shaderVertName), m_shaderNameFrag(shaderFragName) {
}

std::vector<const shaderMetaData *> GShaderPermutationVLK::createMetaArray() {
    return {fragShaderMeta, vertShaderMeta};
}

void GShaderPermutationVLK::compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) {
    auto vertShaderPath = m_shaderConfig.vertexShaderFolder + "/" + m_shaderNameVert;
    auto fragShaderPath = m_shaderConfig.fragmentShaderFolder + "/" + m_shaderNameFrag;

    auto vertShaderCode = readFile("spirv/" + vertShaderPath + ".vert.spv");
    auto fragShaderCode = readFile("spirv/" + fragShaderPath + ".frag.spv");

    vertShaderModule = createShaderModule(vertShaderCode);
    fragShaderModule = createShaderModule(fragShaderCode);

    vertShaderMeta = &shaderMetaInfo.at("./" + vertShaderPath + ".vert.spv");
    fragShaderMeta = &shaderMetaInfo.at("./" + fragShaderPath + ".frag.spv");

    buildLayoutsFromMeta();
}
