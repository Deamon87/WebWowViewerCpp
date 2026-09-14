//
// Created by Deamon on 17.02.23.
//

#include "MaterialBuilderVLK.h"
#include "../GDeviceVulkan.h"
#include "../shaders/GShaderPermutationVLK.h"
#include "ISimpleMaterialVLK.h"

// Specialization constant ID gating the object-id write in shaders (selectionEnabled in
// shaders/slang/common/commonObjectId.slang). Pipelines built when selection is unsupported
// force it to 0 so the outObjectId write is eliminated from the specialized shader.
static constexpr uint32_t SELECTION_ENABLED_SPEC_CONSTANT_ID = 30;

MaterialBuilderVLK::MaterialBuilderVLK(const std::shared_ptr<IDeviceVulkan> &device,
                                       const std::vector<std::string> &shaderFiles, const ShaderConfig &shaderConfig) :
                                       m_device(device), m_shaderFiles(shaderFiles), m_shaderConfig(shaderConfig) {

    m_materialId = 0;
}

MaterialBuilderVLK::MaterialBuilderVLK(
        const std::shared_ptr<IDeviceVulkan> &device,
        const std::shared_ptr<GShaderPermutationVLK> &shader,
        const HPipelineVLK &pipeline,
        const HPipelineVLK &gBufferPipeline,
        const HPipelineVLK &zPrefillPipeline,
        const std::shared_ptr<GPipelineLayoutVLK> &pipelineLayout,
        const PipelineTemplate &pipelineTemplate,
        const std::array<std::shared_ptr<GDescriptorSet>, MAX_SHADER_DESC_SETS> &descriptorSets,
        uint32_t materialId) :
        m_device(device), m_shader(shader), m_pipeline(pipeline), m_gBufferPipeline(gBufferPipeline),
        m_zPrefillPipeline(zPrefillPipeline), m_pipelineTemplate(pipelineTemplate),
        m_descriptorSets(descriptorSets), m_pipelineLayout(pipelineLayout), m_materialId(materialId),
        m_isBuilt(true) {
    // Material is already fully built when using fromMaterial
}

MaterialBuilderVLK &MaterialBuilderVLK::createDescriptorSet(int bindPoint,
                                                            const std::function<void(std::shared_ptr<GDescriptorSet> &ds)> &callback) {

    if (m_isBuilt) {
        // If already built, create descriptor set directly
        auto shaderVLK = m_shader;
        auto ds = std::make_shared<GDescriptorSet>(m_device, shaderVLK->getDescriptorLayout(bindPoint));
        callback(ds);
        m_descriptorSets[bindPoint] = ds;
    } else {
        // Defer creation - store callback for later
        m_descriptorSetCallbacks[bindPoint] = callback;
    }

    return *this;
}

MaterialBuilderVLK &MaterialBuilderVLK::bindDescriptorSet(int bindPoint, const std::shared_ptr<GDescriptorSet> &ds) {
    m_descriptorSets[bindPoint] = ds;
    return *this;
}

MaterialBuilderVLK &MaterialBuilderVLK::createPipeline(const HGVertexBufferBindings &bindings,
                                                       const std::shared_ptr<GRenderPassVLK> &renderPass,
                                                       const PipelineTemplate &pipelineTemplate,
                                                       const std::vector<uint8_t> &specializationConstantsData,
                                                       const std::vector<VkSpecializationMapEntry> &specializationConstantsMetadata) {

    m_bindings = bindings;
    m_renderPass = renderPass;
    m_pipelineTemplate = pipelineTemplate;
    m_specializationConstantsData = specializationConstantsData;
    m_specializationConstantsMetadata = specializationConstantsMetadata;

    // Object-id selection needs per-attachment blend state (independentBlend). When the device
    // lacks it, the forward pass is created without the object-id attachment; force
    // selectionEnabled=0 so shaders skip the outObjectId write entirely.
    // Vulkan ignores specialization entries whose constantID is not declared by the shader
    // module, so this is a no-op for shaders that don't write object ids.
    if (!std::dynamic_pointer_cast<GDeviceVLK>(m_device)->supportsSelection()) {
        uint32_t offset = static_cast<uint32_t>(m_specializationConstantsData.size());
        m_specializationConstantsData.resize(offset + 4, 0);
        m_specializationConstantsMetadata.push_back({SELECTION_ENABLED_SPEC_CONSTANT_ID, offset, 4});
    }

    return *this;
}

MaterialBuilderVLK& MaterialBuilderVLK::createGBufferPipeline(const HGVertexBufferBindings &bindings,
                                          const std::vector<std::string> &shaderFiles, const ShaderConfig &shaderConfig,
                                          const std::shared_ptr<GRenderPassVLK> &renderPass,
                                          const std::vector<uint8_t> &specializationConstantsData,
                                          const std::vector<VkSpecializationMapEntry> &specializationConstantsMetadata) {
    if (m_pipelineTemplate.blendMode >= EGxBlendEnum::GxBlend_Alpha) {
        //GBuffer pipeline is opaque one
        //If blendMode >= EGxBlendEnum::GxBlend_Alpha, it's invalid
        return *this;
    }

    m_hasGBufferPipeline = true;
    m_gBufferShaderFiles = shaderFiles;
    m_gBufferShaderConfig = shaderConfig;
    m_gBufferRenderPass = renderPass;
    m_gBufferSpecializationConstantsData = specializationConstantsData;
    m_gBufferSpecializationConstantsMetadata = specializationConstantsMetadata;

    return *this;
}

MaterialBuilderVLK& MaterialBuilderVLK::createZPrefillPipeline(const HGVertexBufferBindings &bindings,
                                          const std::vector<std::string> &shaderFiles, const ShaderConfig &shaderConfig,
                                          const std::shared_ptr<GRenderPassVLK> &renderPass,
                                          const std::vector<uint8_t> &specializationConstantsData,
                                          const std::vector<VkSpecializationMapEntry> &specializationConstantsMetadata) {
    // Note: depth-only (z-prefill) pipelines are allowed for any blend mode — the blend
    // state is irrelevant in a 0-color-attachment depth pass (e.g. liquid depth pass needs
    // a depth variant of alpha-blended liquid materials)

    m_hasZPrefillPipeline = true;
    m_zPrefillShaderFiles = shaderFiles;
    m_zPrefillShaderConfig = shaderConfig;
    m_zPrefillRenderPass = renderPass;
    m_zPrefillSpecializationConstantsData = specializationConstantsData;
    m_zPrefillSpecializationConstantsMetadata = specializationConstantsMetadata;

    return *this;
}


MaterialBuilderVLK &MaterialBuilderVLK::setMaterialId(uint32_t matId) {
    m_materialId = matId;

    return *this;
}

void MaterialBuilderVLK::ensureBuilt() {
    if (m_isBuilt) {
        return;
    }

    // Collect descriptor set layout overrides from bound descriptor sets
    std::unordered_map<int, const std::shared_ptr<GDescriptorSetLayout>> dsLayoutOverrides = {};
    for (int i = 0; i < MAX_SHADER_DESC_SETS; i++) {
        if (m_descriptorSets[i] != nullptr) {
            dsLayoutOverrides.emplace(i, m_descriptorSets[i]->getDescSetLayout());
        }
    }

    // Create the main shader with descriptor set layout overrides
    m_shader = std::dynamic_pointer_cast<GShaderPermutationVLK>(
        std::dynamic_pointer_cast<GDeviceVLK>(m_device)->getShader(
            m_shaderFiles[0],
            m_shaderFiles[1], 
            m_shaderConfig,
            dsLayoutOverrides)
    );

    m_pipelineLayout = m_shader->getPipelineLayout();

    // Create descriptor sets that were deferred (createDescriptorSet on an unbuilt builder)
    // BEFORE building any secondary (gbuffer/zprefill) shaders. Those shaders may use fewer
    // sets than the main one (e.g. a depth-only variant skipping the texture set), and their
    // pipeline-layout gap check only sees bound sets via the override map — a deferred-created
    // set would otherwise punch a hole in it.
    for (int i = 0; i < MAX_SHADER_DESC_SETS; i++) {
        if (m_descriptorSetCallbacks[i] && m_descriptorSets[i] == nullptr) {
            // Create descriptor set using the stored callback
            auto ds = std::make_shared<GDescriptorSet>(m_device, m_shader->getDescriptorLayout(i));
            m_descriptorSetCallbacks[i](ds);
            m_descriptorSets[i] = ds;
            if (ds->getDescSetLayout() != nullptr) {
                dsLayoutOverrides.emplace(i, ds->getDescSetLayout());
            }
        }
    }

    // Create the main pipeline if requested
    if (m_bindings.has_value()) {
        m_pipeline = std::dynamic_pointer_cast<GDeviceVLK>(m_device)->createPipeline(
            m_bindings.value(),
            m_shader,
            m_pipelineLayout,
            m_renderPass,
            m_pipelineTemplate.element,
            m_pipelineTemplate.backFaceCulling,
            m_pipelineTemplate.triCCW,
            m_pipelineTemplate.blendMode,
            m_pipelineTemplate.depthCulling,
            m_pipelineTemplate.depthWrite,
            m_pipelineTemplate.colorMask,
            m_pipelineTemplate.stencilTestEnable,
            m_pipelineTemplate.stencilWrite,
            m_pipelineTemplate.stencilWriteVal,
            m_specializationConstantsData,
            m_specializationConstantsMetadata
        );
    }

    // Create GBuffer pipeline if requested
    if (m_hasGBufferPipeline && m_bindings.has_value()) {
        auto gbufferShader = std::dynamic_pointer_cast<GShaderPermutationVLK>(
            std::dynamic_pointer_cast<GDeviceVLK>(m_device)->getShader(
                m_gBufferShaderFiles[0],
                m_gBufferShaderFiles[1],
                m_gBufferShaderConfig,
                dsLayoutOverrides)
        );

        m_gBufferPipeline = std::dynamic_pointer_cast<GDeviceVLK>(m_device)->createPipeline(
            m_bindings.value(),
            gbufferShader,
            m_pipelineLayout,
            m_gBufferRenderPass,
            m_pipelineTemplate.element,
            m_pipelineTemplate.backFaceCulling,
            m_pipelineTemplate.triCCW,
            m_pipelineTemplate.blendMode,
            m_pipelineTemplate.depthCulling,
            m_pipelineTemplate.depthWrite,
            m_pipelineTemplate.colorMask,
            m_pipelineTemplate.stencilTestEnable,
            m_pipelineTemplate.stencilWrite,
            m_pipelineTemplate.stencilWriteVal,
            m_gBufferSpecializationConstantsData,
            m_gBufferSpecializationConstantsMetadata
        );
    }

    // Create ZPrefill pipeline if requested
    if (m_hasZPrefillPipeline && m_bindings.has_value()) {
        auto zPrefillShader = std::dynamic_pointer_cast<GShaderPermutationVLK>(
            std::dynamic_pointer_cast<GDeviceVLK>(m_device)->getShader(
                m_zPrefillShaderFiles[0],
                m_zPrefillShaderFiles[1],
                m_zPrefillShaderConfig,
                dsLayoutOverrides)
        );

        m_zPrefillPipeline = std::dynamic_pointer_cast<GDeviceVLK>(m_device)->createPipeline(
            m_bindings.value(),
            zPrefillShader,
            m_pipelineLayout,
            m_zPrefillRenderPass,
            m_pipelineTemplate.element,
            m_pipelineTemplate.backFaceCulling,
            m_pipelineTemplate.triCCW,
            m_pipelineTemplate.blendMode,
            m_pipelineTemplate.depthCulling,
            m_pipelineTemplate.depthWrite,
            m_pipelineTemplate.colorMask,
            m_pipelineTemplate.stencilTestEnable,
            m_pipelineTemplate.stencilWrite,
            m_pipelineTemplate.stencilWriteVal,
            m_zPrefillSpecializationConstantsData,
            m_zPrefillSpecializationConstantsMetadata
        );
    }

    m_isBuilt = true;
}

std::shared_ptr<ISimpleMaterialVLK> MaterialBuilderVLK::toMaterial() {
    ensureBuilt();
    return std::make_shared<ISimpleMaterialVLK>(
        m_shader,
        m_pipelineTemplate,
        m_pipeline,
        m_gBufferPipeline,
        m_zPrefillPipeline,
        m_descriptorSets,
        m_materialId
      );
}
