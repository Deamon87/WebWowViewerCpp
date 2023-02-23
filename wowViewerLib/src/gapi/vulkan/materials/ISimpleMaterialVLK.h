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
    explicit ISimpleMaterialVLK(const HGShaderPermutation &shader,
                                const HPipelineVLK &pipeline,
                                const std::array<std::shared_ptr<GDescriptorSet>, MAX_SHADER_DESC_SETS> &descriptorSets);
    ~ISimpleMaterialVLK() override = default;

    HGShaderPermutation getShader() {
        return m_shader;
    }

    const std::array<std::shared_ptr<GDescriptorSet>, MAX_SHADER_DESC_SETS> &getDescriptorSets() {
        return descriptors;
    }
    HPipelineVLK getPipeline() {
        return m_pipeline;
    }

private:
    std::array<std::shared_ptr<GDescriptorSet>, MAX_SHADER_DESC_SETS> descriptors;

    HGShaderPermutation m_shader;
    HPipelineVLK m_pipeline;
};

typedef std::shared_ptr<ISimpleMaterialVLK> HMaterialVLK;

#endif //AWEBWOWVIEWERCPP_ISIMPLEMATERIALVLK_H
