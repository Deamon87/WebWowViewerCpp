//
// Created by Deamon on 17.02.23.
//

#ifndef AWEBWOWVIEWERCPP_MATERIALBUILDERVLK_H
#define AWEBWOWVIEWERCPP_MATERIALBUILDERVLK_H

#include <vector>
#include <string>
#include <optional>
#include "../../interface/IDevice.h"
#include "ISimpleMaterialVLK.h"
#include "../../../renderer/mapScene/vulkan/materials/IMaterialInstance.h"
#include "../descriptorSets/GDescriptorSet.h"

class MaterialBuilderVLK {
public:
    MaterialBuilderVLK(MaterialBuilderVLK const& ) = delete;
    MaterialBuilderVLK& operator=(MaterialBuilderVLK const& ) = delete;

    static MaterialBuilderVLK fromShader(const std::shared_ptr<IDeviceVulkan> &device,
                                         const std::vector<std::string> &shaderFiles, const ShaderConfig &shaderConfig) {

        return {device, shaderFiles, shaderConfig};
    }

    static MaterialBuilderVLK fromMaterial(const std::shared_ptr<IDeviceVulkan> &device,
                                           const std::shared_ptr<ISimpleMaterialVLK> &materialVlk) {
        return {device, materialVlk->getShader(),
                        materialVlk->getPipeline(),
                        materialVlk->getGBufferPipeline(),
                        materialVlk->getZPrefillPipeline(),
                        materialVlk->getPipeline()->getLayout(),
                        materialVlk->getPipelineTemplate(),
                        toArray(materialVlk->getDescriptorSets()),
                        materialVlk->getMaterialId()
                };
    }
    MaterialBuilderVLK& bindDescriptorSet(int bindPoint, const std::shared_ptr<GDescriptorSet> &ds);
    MaterialBuilderVLK& createDescriptorSet(int bindPoint, const std::function<void(std::shared_ptr<GDescriptorSet> &ds)> &callback);
    MaterialBuilderVLK& createPipeline(const HGVertexBufferBindings &bindings,
                                       const std::shared_ptr<GRenderPassVLK> &renderPass,
                                       const PipelineTemplate &pipelineTemplate,
                                       const std::vector<uint8_t> &specializationConstantsData = {},
                                       const std::vector<VkSpecializationMapEntry> &specializationConstantsMetadata = {});
    MaterialBuilderVLK& createGBufferPipeline(const HGVertexBufferBindings &bindings,
                                       const std::vector<std::string> &shaderFiles, const ShaderConfig &shaderConfig,
                                       const std::shared_ptr<GRenderPassVLK> &renderPass,
                                       const std::vector<uint8_t> &specializationConstantsData = {},
                                       const std::vector<VkSpecializationMapEntry> &specializationConstantsMetadata = {});
    MaterialBuilderVLK& createZPrefillPipeline(const HGVertexBufferBindings &bindings,
                                       const std::vector<std::string> &shaderFiles, const ShaderConfig &shaderConfig,
                                       const std::shared_ptr<GRenderPassVLK> &renderPass,
                                       const std::vector<uint8_t> &specializationConstantsData = {},
                                       const std::vector<VkSpecializationMapEntry> &specializationConstantsMetadata = {});
    MaterialBuilderVLK& setMaterialId(uint32_t matId);
    std::shared_ptr<ISimpleMaterialVLK> toMaterial();

    template<typename T>
    std::shared_ptr<T> toMaterial(const std::function<void(T *)> &initializer) {
        ensureBuilt();
        return std::make_shared<IMaterialInstance<T>>(initializer,
                                   m_shader,
                                   m_pipelineTemplate,
                                   m_pipeline,
                                   m_gBufferPipeline,
                                   m_zPrefillPipeline,
                                   m_descriptorSets,
                                   m_materialId);
    }

    ~MaterialBuilderVLK() = default;
private:
    void ensureBuilt();
    static inline std::array<std::shared_ptr<GDescriptorSet>, MAX_SHADER_DESC_SETS> toArray(const std::vector<std::shared_ptr<GDescriptorSet>> &input) {
        std::array<std::shared_ptr<GDescriptorSet>, MAX_SHADER_DESC_SETS> res;
        for (int i = 0; i < input.size(); i++) {
            res[i] = input[i];
        }
        return res;
    }
    MaterialBuilderVLK(const std::shared_ptr<IDeviceVulkan> &device,
                       const std::vector<std::string> &shaderFiles, const ShaderConfig &shaderConfig);

    MaterialBuilderVLK(const std::shared_ptr<IDeviceVulkan> &device,
                       const std::shared_ptr<GShaderPermutationVLK> &shader,
                       const HPipelineVLK &pipeline,
                       const HPipelineVLK &gBufferPipeline,
                       const HPipelineVLK &zPrefillPipeline,
                       const std::shared_ptr<GPipelineLayoutVLK> &pipelineLayout,
                       const PipelineTemplate &pipelineTemplate,
                       const std::array<std::shared_ptr<GDescriptorSet>, MAX_SHADER_DESC_SETS> &descriptorSets,
                       uint32_t materialId);


private:
    //States
    const std::shared_ptr<IDeviceVulkan> &m_device;

    uint32_t m_materialId;
    std::shared_ptr<GShaderPermutationVLK> m_shader;
    HPipelineVLK m_pipeline;
    HPipelineVLK m_gBufferPipeline = nullptr;
    HPipelineVLK m_zPrefillPipeline = nullptr;
    std::shared_ptr<GPipelineLayoutVLK> m_pipelineLayout;
    PipelineTemplate m_pipelineTemplate;
    std::array<std::shared_ptr<GDescriptorSet>, MAX_SHADER_DESC_SETS> m_descriptorSets;

    // Deferred creation state
    bool m_isBuilt = false;
    std::vector<std::string> m_shaderFiles;
    ShaderConfig m_shaderConfig;
    std::optional<HGVertexBufferBindings> m_bindings;
    std::shared_ptr<GRenderPassVLK> m_renderPass;

    // Descriptor set creation callbacks (for createDescriptorSet)
    std::array<std::function<void(std::shared_ptr<GDescriptorSet> &ds)>, MAX_SHADER_DESC_SETS> m_descriptorSetCallbacks;

    // GBuffer pipeline info (optional)
    bool m_hasGBufferPipeline = false;
    std::vector<std::string> m_gBufferShaderFiles;
    ShaderConfig m_gBufferShaderConfig;
    std::shared_ptr<GRenderPassVLK> m_gBufferRenderPass;

    // ZPrefill pipeline info (optional)
    bool m_hasZPrefillPipeline = false;
    std::vector<std::string> m_zPrefillShaderFiles;
    ShaderConfig m_zPrefillShaderConfig;
    std::shared_ptr<GRenderPassVLK> m_zPrefillRenderPass;

    // Specialization constants (optional)
    std::vector<uint8_t> m_specializationConstantsData;
    std::vector<VkSpecializationMapEntry> m_specializationConstantsMetadata;
    std::vector<uint8_t> m_gBufferSpecializationConstantsData;
    std::vector<VkSpecializationMapEntry> m_gBufferSpecializationConstantsMetadata;
    std::vector<uint8_t> m_zPrefillSpecializationConstantsData;
    std::vector<VkSpecializationMapEntry> m_zPrefillSpecializationConstantsMetadata;
};


#endif //AWEBWOWVIEWERCPP_MATERIALBUILDERVLK_H
