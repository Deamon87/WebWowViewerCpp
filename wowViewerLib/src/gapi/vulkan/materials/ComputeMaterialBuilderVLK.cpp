//
// Created by Deamon on 3/10/2026.
//

#include "ComputeMaterialBuilderVLK.h"
#include "../GDeviceVulkan.h"

// --- ComputePipelineVLK ---

ComputePipelineVLK::ComputePipelineVLK(GDeviceVLK &device, VkPipeline pipeline,
                                       std::shared_ptr<GPipelineLayoutVLK> layout)
    : m_device(device), m_pipeline(pipeline), m_pipelineLayout(std::move(layout)) {
}

ComputePipelineVLK::~ComputePipelineVLK() {
    auto l_pipeline = m_pipeline;
    auto l_vkDevice = m_device.getVkDevice();
    m_device.addDeallocationRecord([l_pipeline, l_vkDevice]() {
        vkDestroyPipeline(l_vkDevice, l_pipeline, nullptr);
    });
}

// --- ComputeMaterialBuilderVLK ---

ComputeMaterialBuilderVLK::ComputeMaterialBuilderVLK(const std::shared_ptr<GDeviceVLK> &device,
                                                     const std::string &shaderName,
                                                     const ComputeShaderConfig &shaderConfig)
    : m_device(device), m_shaderName(shaderName), m_shaderConfig(shaderConfig) {
}

ComputeMaterialBuilderVLK &ComputeMaterialBuilderVLK::bindDescriptorSet(int bindPoint,
                                                                         const std::shared_ptr<GDescriptorSet> &ds) {
    m_descriptorSets[bindPoint] = ds;
    return *this;
}

ComputeMaterialBuilderVLK &ComputeMaterialBuilderVLK::createDescriptorSet(int bindPoint,
                                                                            const std::function<void(std::shared_ptr<GDescriptorSet> &ds)> &callback) {
    if (m_isBuilt) {
        auto ds = std::make_shared<GDescriptorSet>(m_device, m_shader->getDescriptorLayout(bindPoint));
        callback(ds);
        m_descriptorSets[bindPoint] = ds;
    } else {
        m_descriptorSetCallbacks[bindPoint] = callback;
    }
    return *this;
}

void ComputeMaterialBuilderVLK::ensureBuilt() {
    if (m_isBuilt)
        return;

    // Collect descriptor set layout overrides from pre-bound descriptor sets
    std::unordered_map<int, const std::shared_ptr<GDescriptorSetLayout>> dsLayoutOverrides;
    for (int i = 0; i < MAX_SHADER_DESC_SETS; i++) {
        if (m_descriptorSets[i] != nullptr) {
            dsLayoutOverrides.emplace(i, m_descriptorSets[i]->getDescSetLayout());
        }
    }

    // Create and compile the compute shader
    m_shader = std::make_shared<GComputeShaderVLK>(m_shaderName, m_device, m_shaderConfig, dsLayoutOverrides);
    m_shader->compile();

    // Create compute pipeline via device (cached)
    m_pipeline = m_device->createComputePipeline(m_shader, m_shader->getPipelineLayout());

    // Create any deferred descriptor sets
    for (int i = 0; i < MAX_SHADER_DESC_SETS; i++) {
        if (m_descriptorSetCallbacks[i] && m_descriptorSets[i] == nullptr) {
            auto ds = std::make_shared<GDescriptorSet>(m_device, m_shader->getDescriptorLayout(i));
            m_descriptorSetCallbacks[i](ds);
            m_descriptorSets[i] = ds;
        }
    }

    m_isBuilt = true;
}

std::shared_ptr<ComputeMaterialVLK> ComputeMaterialBuilderVLK::toMaterial() {
    ensureBuilt();

    auto material = std::make_shared<ComputeMaterialVLK>();
    material->shader = m_shader;
    material->pipeline = m_pipeline;
    for (int i = 0; i < MAX_SHADER_DESC_SETS; i++) {
        if (m_descriptorSets[i]) {
            material->descriptorSets.push_back(m_descriptorSets[i]);
        }
    }
    return material;
}
