//
// Created by Deamon on 9/26/2019.
//

#ifndef AWEBWOWVIEWERCPP_GPIPELINEVLK_H
#define AWEBWOWVIEWERCPP_GPIPELINEVLK_H

#include "../interface/IDevice.h"
#include "GDeviceVulkan.h"
#include "GVertexBufferBindingsVLK.h"

class GPipelineVLK {
    friend class GDeviceVLK;
public:
    explicit GPipelineVLK(IDevice &m_device,
                          HGVertexBufferBindings m_bindings,
                          std::shared_ptr<GRenderPassVLK> renderPass,
                          HGShaderPermutation shader,
                          DrawElementMode element,
                          int8_t backFaceCulling,
                          int8_t triCCW,
                          EGxBlendEnum blendMode,
                          int8_t depthCulling,
                          int8_t depthWrite);
    ~GPipelineVLK();

    void createPipeline(
        GShaderPermutationVLK *shaderVLK,
        std::shared_ptr<GRenderPassVLK> renderPass,
        DrawElementMode m_element,
        int8_t m_backFaceCulling,
        int8_t m_triCCW,
        EGxBlendEnum m_blendMode,
        int8_t m_depthCulling,
        int8_t m_depthWrite,

        const std::vector<VkVertexInputBindingDescription> &vertexBindingDescriptions,
        const std::vector<VkVertexInputAttributeDescription> &vertexAttributeDescriptions);
private:
    GDeviceVLK &m_device;

    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
};


#endif //AWEBWOWVIEWERCPP_GPIPELINEVLK_H
