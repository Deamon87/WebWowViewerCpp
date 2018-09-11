//
// Created by deamon on 05.06.18.
//

#include "GMeshGL4x.h"

GMeshGL4x::GMeshGL4x(IDevice &device,
             const gMeshTemplate &meshTemplate
) : m_device(device), m_shader(meshTemplate.shader), m_meshType(meshTemplate.meshType) {

    m_bindings = meshTemplate.bindings;

    m_depthWrite = (int8_t) (meshTemplate.depthWrite ? 1u : 0u);
    m_depthCulling = (int8_t) (meshTemplate.depthCulling ? 1 : 0);
    m_backFaceCulling = (int8_t) (meshTemplate.backFaceCulling ? 1 : 0);
    m_triCCW = meshTemplate.triCCW;

    m_colorMask = meshTemplate.colorMask;

    m_blendMode = meshTemplate.blendMode;
    m_isTransparent = m_blendMode > EGxBlendEnum::GxBlend_AlphaKey || !m_depthWrite ;

    m_start = meshTemplate.start;
    m_end = meshTemplate.end;
    m_element = meshTemplate.element;
    m_textureCount = meshTemplate.textureCount;

    m_texture = meshTemplate.texture;

    m_vertexUniformBuffer[0] = meshTemplate.vertexBuffers[0];
    m_vertexUniformBuffer[1] = meshTemplate.vertexBuffers[1];
    m_vertexUniformBuffer[2] = meshTemplate.vertexBuffers[2];

    m_fragmentUniformBuffer[0] = meshTemplate.fragmentBuffers[0];
    m_fragmentUniformBuffer[1] = meshTemplate.fragmentBuffers[1];
    m_fragmentUniformBuffer[2] = meshTemplate.fragmentBuffers[2];

}
GMeshGL4x::~GMeshGL4x() {

}

HGUniformBuffer GMeshGL4x::getVertexUniformBuffer(int slot) {
    return m_vertexUniformBuffer[slot];
}

HGUniformBuffer GMeshGL4x::getFragmentUniformBuffer(int slot) {
    return m_fragmentUniformBuffer[slot];
}

EGxBlendEnum GMeshGL4x::getGxBlendMode() { return m_blendMode; }

bool GMeshGL4x::getIsTransparent() { return m_isTransparent; }

MeshType GMeshGL4x::getMeshType() {
    return m_meshType;
}

void GMeshGL4x::setRenderOrder(int renderOrder) {
    m_renderOrder = renderOrder;
}

void GMeshGL4x::setEnd(int end) {m_end = end; }


