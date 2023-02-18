//
// Created by Deamon on 17.02.23.
//

#include "MaterialBuilderVLK.h"
#include "../GDeviceVulkan.h"
#include "../shaders/GShaderPermutationVLK.h"
#include "ISimpleMaterialVLK.h"

MaterialBuilderVLK::MaterialBuilderVLK(const std::shared_ptr<IDeviceVulkan> &device,
                                       const std::vector<std::string> &shaderFiles) : m_device(device) {

    m_shader =  std::dynamic_pointer_cast<GDeviceVLK>(device)->getShader(
        shaderFiles[0],
        shaderFiles[1], nullptr);
}

MaterialBuilderVLK &MaterialBuilderVLK::createDescriptorSet(int bindPoint,
                                                            const std::function<void(std::shared_ptr<GDescriptorSet> &ds)> &callback) {

    auto shaderVLK = std::dynamic_pointer_cast<GShaderPermutationVLK>(m_shader);

    auto ds = std::make_shared<GDescriptorSet>(m_device, shaderVLK->getDescriptorLayout(bindPoint));
    callback(ds);

    descriptorSets[bindPoint] = ds;

    return *this;
}

MaterialBuilderVLK &MaterialBuilderVLK::bindDescriptorSet(int bindPoint, std::shared_ptr<GDescriptorSet> &ds) {
    //TODO: check DS layout compatibility

    descriptorSets[bindPoint] = ds;

    return *this;
}

std::shared_ptr<ISimpleMaterialVLK> MaterialBuilderVLK::toMaterial() {
    return std::make_shared<ISimpleMaterialVLK>(
        m_shader,
        descriptorSets
      );
}
