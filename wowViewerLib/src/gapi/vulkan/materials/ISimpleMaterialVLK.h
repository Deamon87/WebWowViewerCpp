//
// Created by Deamon on 29.12.22.
//

#ifndef AWEBWOWVIEWERCPP_ISIMPLEMATERIALVLK_H
#define AWEBWOWVIEWERCPP_ISIMPLEMATERIALVLK_H


#include <vector>
#include <memory>
#include "../../interface/materials/IMaterial.h"
#include "../descriptorSets/GDescriptorSet.h"
#include "../textures/GTextureVLK.h"
#include "../GDeviceVulkan.h"

class ISimpleMaterialVLK : public IMaterial, public std::enable_shared_from_this<ISimpleMaterialVLK> {
public:
    explicit ISimpleMaterialVLK(const std::shared_ptr<GShaderPermutationVLK> &shader,
                                const PipelineTemplate &pipelineTemplate,
                                const HPipelineVLK &pipeline,
                                const std::array<std::shared_ptr<GDescriptorSet>, MAX_SHADER_DESC_SETS> &descriptorSets);
    ~ISimpleMaterialVLK() override = default;

    const std::shared_ptr<GShaderPermutationVLK> &getShader() const {
        return m_shader;
    }

    const std::vector<std::shared_ptr<GDescriptorSet>> &getDescriptorSets() {
        return descriptors;
    }
    const HPipelineVLK &getPipeline() const {
        return m_pipeline;
    }
    const PipelineTemplate &getPipelineTemplate() const {
        return m_pipelineTemplate;
    }
    std::shared_ptr<GPipelineVLK> getPipeLineForRenderPass(const std::shared_ptr<GRenderPassVLK> &renderPass);
    EGxBlendEnum getBlendMode();

private:
    std::vector<std::shared_ptr<GDescriptorSet>> descriptors;

    std::shared_ptr<GShaderPermutationVLK> m_shader;
    HPipelineVLK m_pipeline;
    PipelineTemplate m_pipelineTemplate;
};

typedef std::shared_ptr<ISimpleMaterialVLK> HMaterialVLK;

#endif //AWEBWOWVIEWERCPP_ISIMPLEMATERIALVLK_H
