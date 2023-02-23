//
// Created by Deamon on 29.12.22.
//

#include "ISimpleMaterialVLK.h"
#include "../shaders/GShaderPermutationVLK.h"
#include "../textures/GTextureVLK.h"
ISimpleMaterialVLK::ISimpleMaterialVLK(const HGShaderPermutation &shader,
                                       const HPipelineVLK &pipeline,
                                       const std::array<std::shared_ptr<GDescriptorSet>, MAX_SHADER_DESC_SETS> &descriptorSets) :
                                       m_shader(shader), m_pipeline(pipeline), descriptors(descriptorSets) {

}
