//
// Created by deamon on 05.06.18.
//

#include <iostream>
#include <array>
#include "GMeshVLK.h"
#include "../textures/GTextureVLK.h"
#include "../shaders/GShaderPermutationVLK.h"

GMeshVLK::GMeshVLK(IDevice &device,
            const gMeshTemplate &meshTemplate,
            const HMaterialVLK &material
) : m_device(dynamic_cast<GDeviceVLK &>(device)), m_meshType(meshTemplate.meshType), m_material(material) {

    m_bindings = meshTemplate.bindings;

    m_depthWrite = (int8_t) (meshTemplate.depthWrite ? 1u : 0u);
    m_depthCulling = (int8_t) (meshTemplate.depthCulling ? 1 : 0);
    m_backFaceCulling = (int8_t) (meshTemplate.backFaceCulling ? 1 : 0);
    m_triCCW = meshTemplate.triCCW;

    m_isScissorsEnabled = meshTemplate.scissorEnabled ? 1 : 0;
    if (m_isScissorsEnabled) {
        m_scissorSize = meshTemplate.scissorSize;
        m_scissorOffset = meshTemplate.scissorOffset;
    }

    m_colorMask = meshTemplate.colorMask;

    m_blendMode = meshTemplate.blendMode;

    m_start = meshTemplate.start;
    m_end = meshTemplate.end;
    m_element = meshTemplate.element;
}

//Works under assumption that meshes do not change the renderpass, on which they are rendered, too often
std::shared_ptr<GPipelineVLK> GMeshVLK::getPipeLineForRenderPass(std::shared_ptr<GRenderPassVLK> renderPass, bool invertedZ) {
    if (m_lastRenderPass != renderPass || m_lastInvertedZ != invertedZ) {
        m_lastPipelineForRenderPass = m_device.createPipeline(m_bindings,
                                                              m_material->getShader(), renderPass, m_element,
                                                              m_backFaceCulling, m_triCCW,
                                                              m_blendMode, m_depthCulling,
                                                              m_depthWrite, invertedZ);
        m_lastRenderPass = renderPass;
        m_lastInvertedZ = invertedZ;
    }

    return m_lastPipelineForRenderPass;
}




GMeshVLK::~GMeshVLK() {

}

bool GMeshVLK::getIsTransparent() { return m_isTransparent; }

MeshType GMeshVLK::getMeshType() {
    return m_meshType;
}
