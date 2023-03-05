//
// Created by Deamon on 17.02.23.
//

#ifndef AWEBWOWVIEWERCPP_MATERIALBUILDERVLK_H
#define AWEBWOWVIEWERCPP_MATERIALBUILDERVLK_H

#include <vector>
#include <string>
#include "../../interface/IDevice.h"
#include "ISimpleMaterialVLK.h"
#include "../descriptorSets/GDescriptorSet.h"

class MaterialBuilderVLK {
public:
    MaterialBuilderVLK(MaterialBuilderVLK const& ) = delete;
    MaterialBuilderVLK& operator=(MaterialBuilderVLK const& ) = delete;

    static MaterialBuilderVLK fromShader(const std::shared_ptr<IDeviceVulkan> &device,
                                         const std::vector<std::string> &shaderFiles) {
        return {device, shaderFiles};
    }

    MaterialBuilderVLK& bindDescriptorSet(int bindPoint, std::shared_ptr<GDescriptorSet> &ds);
    MaterialBuilderVLK& createDescriptorSet(int bindPoint, const std::function<void(std::shared_ptr<GDescriptorSet> &ds)> &callback);
    MaterialBuilderVLK& createPipeline(const HGVertexBufferBindings &bindings,
                                       const std::shared_ptr<GRenderPassVLK> &renderPass,
                                       const PipelineTemplate &pipelineTemplate);
    std::shared_ptr<ISimpleMaterialVLK> toMaterial();

    ~MaterialBuilderVLK() = default;
private:
    MaterialBuilderVLK(const std::shared_ptr<IDeviceVulkan> &device,
                       const std::vector<std::string> &shaderFiles);


private:
    //States
    const std::shared_ptr<IDeviceVulkan> &m_device;

    HGShaderPermutation m_shader;
    HPipelineVLK m_pipeline;
    PipelineTemplate m_pipelineTemplate;
    std::array<std::shared_ptr<GDescriptorSet>, MAX_SHADER_DESC_SETS> descriptorSets;
};


#endif //AWEBWOWVIEWERCPP_MATERIALBUILDERVLK_H
