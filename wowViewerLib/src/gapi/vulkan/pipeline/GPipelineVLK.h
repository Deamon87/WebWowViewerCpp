//
// Created by Deamon on 9/26/2019.
//

#ifndef AWEBWOWVIEWERCPP_GPIPELINEVLK_H
#define AWEBWOWVIEWERCPP_GPIPELINEVLK_H

class GPipelineLayoutVLK;

#include "../../interface/IDevice.h"
#include "../GDeviceVulkan.h"
#include "../GVertexBufferBindingsVLK.h"
#include "GPipelineLayoutVLK.h"

class GPipelineVLK {
    friend class GDeviceVLK;
public:
    explicit GPipelineVLK(IDevice &m_device,
                          const HGVertexBufferBindings &m_bindings,
                          const std::shared_ptr<GRenderPassVLK> &renderPass,
                          const std::shared_ptr<GPipelineLayoutVLK> &pipelineLayout,
                          const HGShaderPermutation &shader,
                          DrawElementMode element,
                          bool backFaceCulling,
                          bool triCCW,
                          EGxBlendEnum blendMode,
                          bool depthCulling,
                          bool depthWrite);
    ~GPipelineVLK();

    void createPipeline(
        GShaderPermutationVLK *shaderVLK,
        const std::shared_ptr<GRenderPassVLK> &renderPass,
        DrawElementMode m_element,
        bool m_backFaceCulling,
        bool m_triCCW,
        EGxBlendEnum m_blendMode,
        bool m_depthCulling,
        bool m_depthWrite,

        const std::vector<VkVertexInputBindingDescription> &vertexBindingDescriptions,
        const std::vector<VkVertexInputAttributeDescription> &vertexAttributeDescriptions);

    std::shared_ptr<GPipelineLayoutVLK> getLayout() { return m_pipelineLayout; };
    VkPipeline getPipeline() { return graphicsPipeline; };
    bool getIsTransparent() const {
        return m_isTransparent;
    }

    const std::shared_ptr<GRenderPassVLK> getRenderPass() {
        return m_renderPass;
    }
private:
    GDeviceVLK &m_device;

    std::shared_ptr<GPipelineLayoutVLK> m_pipelineLayout;
    VkPipeline graphicsPipeline;

    std::shared_ptr<GRenderPassVLK> m_renderPass;

    bool m_isTransparent = false;
};


#endif //AWEBWOWVIEWERCPP_GPIPELINEVLK_H
