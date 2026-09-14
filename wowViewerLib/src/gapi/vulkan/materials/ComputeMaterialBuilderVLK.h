//
// Created by Deamon on 3/10/2026.
//

#ifndef AWEBWOWVIEWERCPP_COMPUTEMATERIALBUILDERVLK_H
#define AWEBWOWVIEWERCPP_COMPUTEMATERIALBUILDERVLK_H

#include <vector>
#include <string>
#include <functional>
#include "../shaders/GComputeShaderVLK.h"
#include "../descriptorSets/GDescriptorSet.h"
#include "../pipeline/GPipelineLayoutVLK.h"

class GDeviceVLK;

class ComputePipelineVLK {
public:
    ComputePipelineVLK(GDeviceVLK &device, VkPipeline pipeline,
                       std::shared_ptr<GPipelineLayoutVLK> layout);
    ~ComputePipelineVLK();

    VkPipeline getPipeline() { return m_pipeline; }
    std::shared_ptr<GPipelineLayoutVLK> getLayout() { return m_pipelineLayout; }

private:
    GDeviceVLK &m_device;
    VkPipeline m_pipeline;
    std::shared_ptr<GPipelineLayoutVLK> m_pipelineLayout;
};

typedef std::shared_ptr<ComputePipelineVLK> HComputePipelineVLK;

struct ComputeMaterialVLK {
    std::shared_ptr<GComputeShaderVLK> shader;
    HComputePipelineVLK pipeline;
    std::vector<std::shared_ptr<GDescriptorSet>> descriptorSets;
};

class ComputeMaterialBuilderVLK {
public:
    ComputeMaterialBuilderVLK(ComputeMaterialBuilderVLK const &) = delete;
    ComputeMaterialBuilderVLK &operator=(ComputeMaterialBuilderVLK const &) = delete;

    static ComputeMaterialBuilderVLK fromShader(const std::shared_ptr<GDeviceVLK> &device,
                                                const std::string &shaderName,
                                                const ComputeShaderConfig &shaderConfig) {
        return {device, shaderName, shaderConfig};
    }

    ComputeMaterialBuilderVLK &bindDescriptorSet(int bindPoint, const std::shared_ptr<GDescriptorSet> &ds);
    ComputeMaterialBuilderVLK &createDescriptorSet(int bindPoint,
                                                    const std::function<void(std::shared_ptr<GDescriptorSet> &ds)> &callback);

    std::shared_ptr<ComputeMaterialVLK> toMaterial();

    ~ComputeMaterialBuilderVLK() = default;

private:
    ComputeMaterialBuilderVLK(const std::shared_ptr<GDeviceVLK> &device,
                              const std::string &shaderName,
                              const ComputeShaderConfig &shaderConfig);

    void ensureBuilt();

    std::shared_ptr<GDeviceVLK> m_device;
    std::string m_shaderName;
    ComputeShaderConfig m_shaderConfig;

    std::shared_ptr<GComputeShaderVLK> m_shader;
    HComputePipelineVLK m_pipeline;
    std::array<std::shared_ptr<GDescriptorSet>, MAX_SHADER_DESC_SETS> m_descriptorSets = {};
    std::array<std::function<void(std::shared_ptr<GDescriptorSet> &ds)>, MAX_SHADER_DESC_SETS> m_descriptorSetCallbacks = {};

    bool m_isBuilt = false;
};

#endif //AWEBWOWVIEWERCPP_COMPUTEMATERIALBUILDERVLK_H
