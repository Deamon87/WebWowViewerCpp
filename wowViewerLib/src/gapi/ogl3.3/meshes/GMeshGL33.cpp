//
// Created by deamon on 05.06.18.
//

#include <iostream>
#include "GMeshGL33.h"

GMeshGL33::GMeshGL33(IDevice &device,
             const gMeshTemplate &meshTemplate
) : m_device(device), m_shader(meshTemplate.shader), m_meshType(meshTemplate.meshType) {

    m_bindings = meshTemplate.bindings;

    m_depthWrite = (int8_t) (meshTemplate.depthWrite ? 1u : 0u);
    m_depthCulling = (int8_t) (meshTemplate.depthCulling ? 1 : 0);
    m_backFaceCulling = (int8_t) (meshTemplate.backFaceCulling ? 1 : 0);
    m_triCCW = meshTemplate.triCCW;

    m_isSkyBox = meshTemplate.skybox;

    m_colorMask = meshTemplate.colorMask;

    m_blendMode = meshTemplate.blendMode;
    m_isTransparent = m_blendMode > EGxBlendEnum::GxBlend_AlphaKey || !m_depthWrite ;

    m_start = meshTemplate.start;
    m_end = meshTemplate.end;
    switch (meshTemplate.element) {
        case DrawElementMode::TRIANGLES:
            m_element = GL_TRIANGLES;
            break;
        case DrawElementMode::TRIANGLE_STRIP:
            m_element = GL_TRIANGLE_STRIP;
            break;
        default:
            throw new std::runtime_error("unknown DrawElementMode");
    }

    m_textureCount = meshTemplate.textureCount;

    m_texture = meshTemplate.texture;

    m_vertexUniformBuffer[0] = meshTemplate.vertexBuffers[0];
    m_vertexUniformBuffer[1] = meshTemplate.vertexBuffers[1];
    m_vertexUniformBuffer[2] = meshTemplate.vertexBuffers[2];

    m_fragmentUniformBuffer[0] = meshTemplate.fragmentBuffers[0];
    m_fragmentUniformBuffer[1] = meshTemplate.fragmentBuffers[1];
    m_fragmentUniformBuffer[2] = meshTemplate.fragmentBuffers[2];

}
GMeshGL33::~GMeshGL33() {

}

HGUniformBuffer GMeshGL33::getUniformBuffer(int slot) {
    return m_vertexUniformBuffer[slot];
}


EGxBlendEnum GMeshGL33::getGxBlendMode() { return m_blendMode; }

bool GMeshGL33::getIsTransparent() { return m_isTransparent; }

MeshType GMeshGL33::getMeshType() {
    return m_meshType;
}

void GMeshGL33::setRenderOrder(int renderOrder) {
    m_renderOrder = renderOrder;
}

void GMeshGL33::setStart(int start) {m_start = start; }
void GMeshGL33::setEnd(int end) {m_end = end; }


