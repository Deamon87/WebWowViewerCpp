//
// Created by Deamon on 29.12.22.
//

#include "ISimpleMaterialVLK.h"
#include "../shaders/GShaderPermutationVLK.h"
#include "../textures/GTextureVLK.h"
#include "../GPipelineVLK.h"


ISimpleMaterialVLK::ISimpleMaterialVLK(const HGShaderPermutation &shader,
                                       const PipelineTemplate &pipelineTemplate,
                                       const HPipelineVLK &pipeline,
                                       const std::array<std::shared_ptr<GDescriptorSet>, MAX_SHADER_DESC_SETS> &descriptorSets) :
                                       m_shader(shader), m_pipelineTemplate(pipelineTemplate),
                                       m_pipeline(pipeline), descriptors(descriptorSets) {

}

//Works under assumption that meshes do not change the renderpass, on which they are rendered, too often
std::shared_ptr<GPipelineVLK> ISimpleMaterialVLK::getPipeLineForRenderPass(const std::shared_ptr<GRenderPassVLK> &renderPass) {

//TODO: Come up with more elegant way of recreating pipeline, when the renderPass changes(for example, when multiview is activated)

//    if (m_pipeline->getRenderPass() != renderPass) {
//        m_pipeline = m_device.createPipeline(m_bindings,
//                        getShader(), renderPass, material()->getPipeline(),
//                        m_backFaceCulling, m_triCCW,
//                        m_blendMode, m_depthCulling,
//                        m_depthWrite);
//
//    }

    return m_pipeline;
}
