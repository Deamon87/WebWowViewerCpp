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
                          HGVertexBufferBindings &m_bindings,
                          HGShaderPermutation &shader,
                          DrawElementMode element,
                          int8_t backFaceCulling,
                          int8_t triCCW,
                          EGxBlendEnum blendMode,
                          int8_t depthCulling,
                          int8_t depthWrite,
                          int8_t skyBoxMode);
    ~GPipelineVLK();

    void createPipeline(
        const std::vector<VkVertexInputBindingDescription> &vertexBindingDescriptions,
        const std::vector<VkVertexInputAttributeDescription> &vertexAttributeDescriptions);

    void recreatePipeline();
private:
    GDeviceVLK &m_device;

    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    HGVertexBufferBindings m_bindings,
    HGShaderPermutation m_shader;
    DrawElementMode m_element;
    int8_t m_backFaceCulling;
    int8_t m_triCCW;
    EGxBlendEnum m_blendMode;
    int8_t m_depthCulling;
    int8_t m_depthWrite;
    int8_t m_skyBoxMod;
};


#endif //AWEBWOWVIEWERCPP_GPIPELINEVLK_H
