//
// Created by Deamon on 3/10/2026.
//

#include <iostream>
#include "GComputeShaderVLK.h"
#include <ShaderDefinitions.h>

GComputeShaderVLK::GComputeShaderVLK(const std::string &shaderName,
                                     const std::shared_ptr<GDeviceVLK> &device,
                                     const ComputeShaderConfig &shaderConf,
                                     const std::unordered_map<int, const std::shared_ptr<GDescriptorSetLayout>> &dsLayoutOverrides)
    : GShaderPermutationVLKBase(shaderName, device, shaderConf.typeOverrides, dsLayoutOverrides),
      m_computeConfig(shaderConf), m_shaderName(shaderName) {
}

std::vector<const shaderMetaData *> GComputeShaderVLK::createMetaArray() {
    return {m_computeMeta};
}

void GComputeShaderVLK::compile() {
    auto computeShaderPath = m_computeConfig.computeShaderFolder + "/" + m_shaderName;

    auto computeShaderCode = readFile("spirv/" + computeShaderPath + ".comp.spv");
    m_computeModule = createShaderModule(computeShaderCode);
    m_computeMeta = &shaderMetaInfo.at("./" + computeShaderPath + ".comp.spv");

    buildLayoutsFromMeta();
}
