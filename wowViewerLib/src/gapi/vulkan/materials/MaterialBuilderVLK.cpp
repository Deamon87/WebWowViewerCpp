//
// Created by Deamon on 17.02.23.
//

#include "MaterialBuilderVLK.h"
#include "../GDeviceVulkan.h"
#include "../shaders/GShaderPermutationVLK.h"
#include "ISimpleMaterialVLK.h"

MaterialBuilderVLK::MaterialBuilderVLK(const std::shared_ptr<IDeviceVulkan> &device,
                                       const std::vector<std::string> &shaderFiles, const ShaderConfig &shaderConfig) : m_device(device) {

    m_shader =  std::dynamic_pointer_cast<GDeviceVLK>(device)->getShader(
        shaderFiles[0],
        shaderFiles[1], shaderConfig);
}

MaterialBuilderVLK::MaterialBuilderVLK(
        const std::shared_ptr<IDeviceVulkan> &device,
        const HGShaderPermutation &shader,
        const HPipelineVLK &pipeline,
        const PipelineTemplate &pipelineTemplate,
        const std::array<std::shared_ptr<GDescriptorSet>, MAX_SHADER_DESC_SETS> &descriptorSets) :
        m_device(device), m_shader(shader), m_pipeline(pipeline), m_pipelineTemplate(pipelineTemplate),
        m_descriptorSets(descriptorSets) {

}

MaterialBuilderVLK &MaterialBuilderVLK::createDescriptorSet(int bindPoint,
                                                            const std::function<void(std::shared_ptr<GDescriptorSet> &ds)> &callback) {

    auto shaderVLK = std::dynamic_pointer_cast<GShaderPermutationVLK>(m_shader);

    auto ds = std::make_shared<GDescriptorSet>(m_device, shaderVLK->getDescriptorLayout(bindPoint));
    callback(ds);

    m_descriptorSets[bindPoint] = ds;

    return *this;
}

MaterialBuilderVLK &MaterialBuilderVLK::bindDescriptorSet(int bindPoint, std::shared_ptr<GDescriptorSet> &ds) {
    //TODO: check DS layout compatibility

    m_descriptorSets[bindPoint] = ds;

    return *this;
}

MaterialBuilderVLK &MaterialBuilderVLK::createPipeline(const HGVertexBufferBindings &bindings,
                                                       const std::shared_ptr<GRenderPassVLK> &renderPass,
                                                       const PipelineTemplate &pipelineTemplate) {

    m_pipelineTemplate = pipelineTemplate;
    m_pipeline = std::dynamic_pointer_cast<GDeviceVLK>(m_device)->createPipeline(
        bindings,
        m_shader,
        renderPass,
        pipelineTemplate.element,
        pipelineTemplate.backFaceCulling,
        pipelineTemplate.triCCW,
        pipelineTemplate.blendMode,
        pipelineTemplate.depthCulling,
        pipelineTemplate.depthWrite
    );

    return *this;
}

std::shared_ptr<ISimpleMaterialVLK> MaterialBuilderVLK::toMaterial() {
    return std::make_shared<ISimpleMaterialVLK>(
        m_shader,
        m_pipelineTemplate,
        m_pipeline,
        m_descriptorSets
      );
}
