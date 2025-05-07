//
// Created by Deamon on 17.02.23.
//

#include "MaterialBuilderVLK.h"
#include "../GDeviceVulkan.h"
#include "../shaders/GShaderPermutationVLK.h"
#include "ISimpleMaterialVLK.h"

MaterialBuilderVLK::MaterialBuilderVLK(const std::shared_ptr<IDeviceVulkan> &device,
                                       const std::vector<std::string> &shaderFiles, const ShaderConfig &shaderConfig,
                                       const std::unordered_map<int, const std::shared_ptr<GDescriptorSetLayout>> &dsLayoutOverrides) :
                                       m_device(device), m_dsLayoutOverrides(dsLayoutOverrides) {

    m_shader = std::dynamic_pointer_cast<GShaderPermutationVLK>(std::dynamic_pointer_cast<GDeviceVLK>(device)->getShader(
        shaderFiles[0],
        shaderFiles[1], shaderConfig,
        dsLayoutOverrides)
    );

    m_materialId = 0;

    m_pipelineLayout = m_shader->getPipelineLayout();
}

MaterialBuilderVLK::MaterialBuilderVLK(
        const std::shared_ptr<IDeviceVulkan> &device,
        const std::shared_ptr<GShaderPermutationVLK> &shader,
        const HPipelineVLK &pipeline,
        const HPipelineVLK &gBufferPipeline,
        const std::shared_ptr<GPipelineLayoutVLK> &pipelineLayout,
        const PipelineTemplate &pipelineTemplate,
        const std::array<std::shared_ptr<GDescriptorSet>, MAX_SHADER_DESC_SETS> &descriptorSets,
        uint32_t materialId) :
        m_device(device), m_shader(shader), m_pipeline(pipeline), m_gBufferPipeline(gBufferPipeline),
        m_pipelineTemplate(pipelineTemplate),
        m_descriptorSets(descriptorSets), m_pipelineLayout(pipelineLayout), m_materialId(materialId) {

}

MaterialBuilderVLK &MaterialBuilderVLK::createDescriptorSet(int bindPoint,
                                                            const std::function<void(std::shared_ptr<GDescriptorSet> &ds)> &callback) {

    auto shaderVLK = m_shader;

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
        m_pipelineLayout,
        renderPass,
        pipelineTemplate.element,
        pipelineTemplate.backFaceCulling,
        pipelineTemplate.triCCW,
        pipelineTemplate.blendMode,
        pipelineTemplate.depthCulling,
        pipelineTemplate.depthWrite,
        pipelineTemplate.colorMask,
        pipelineTemplate.stencilTestEnable,
        pipelineTemplate.stencilWrite,
        pipelineTemplate.stencilWriteVal
    );

    return *this;
}

MaterialBuilderVLK& MaterialBuilderVLK::createGBufferPipeline(const HGVertexBufferBindings &bindings,
                                          const std::vector<std::string> &shaderFiles, const ShaderConfig &shaderConfig,
                                          const std::shared_ptr<GRenderPassVLK> &renderPass) {
    if (m_pipelineTemplate.blendMode >= EGxBlendEnum::GxBlend_Alpha) {
        return *this;
    }

    auto gbufferShader = std::dynamic_pointer_cast<GShaderPermutationVLK>(std::dynamic_pointer_cast<GDeviceVLK>(m_device)->getShader(
        shaderFiles[0],
        shaderFiles[1], shaderConfig, m_dsLayoutOverrides));


    m_gBufferPipeline = std::dynamic_pointer_cast<GDeviceVLK>(m_device)->createPipeline(
        bindings,
        gbufferShader,
        m_pipelineLayout,
        renderPass,
        m_pipelineTemplate.element,
        m_pipelineTemplate.backFaceCulling,
        m_pipelineTemplate.triCCW,
        m_pipelineTemplate.blendMode,
        m_pipelineTemplate.depthCulling,
        m_pipelineTemplate.depthWrite,
        m_pipelineTemplate.colorMask,
        m_pipelineTemplate.stencilTestEnable,
        m_pipelineTemplate.stencilWrite,
        m_pipelineTemplate.stencilWriteVal
    );

    return *this;
}


MaterialBuilderVLK &MaterialBuilderVLK::setMaterialId(uint32_t matId) {
    m_materialId = matId;

    return *this;
}

std::shared_ptr<ISimpleMaterialVLK> MaterialBuilderVLK::toMaterial() {
    return std::make_shared<ISimpleMaterialVLK>(
        m_shader,
        m_pipelineTemplate,
        m_pipeline,
        m_gBufferPipeline,
        m_descriptorSets,
        m_materialId
      );
}
