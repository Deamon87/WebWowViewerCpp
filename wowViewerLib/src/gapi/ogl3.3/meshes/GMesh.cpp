//
// Created by deamon on 05.06.18.
//

#include "GMesh.h"

GMesh::GMesh(GDevice &device,
             const gMeshTemplate &meshTemplate
) : m_device(device), m_bindings(meshTemplate.bindings), m_shader(meshTemplate.shader), m_meshType(meshTemplate.meshType) {

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
GMesh::~GMesh() {

}

HGUniformBuffer GMesh::getVertexUniformBuffer(int slot) {
    return m_vertexUniformBuffer[slot];
}

HGUniformBuffer GMesh::getFragmentUniformBuffer(int slot) {
    return m_fragmentUniformBuffer[slot];
}

EGxBlendEnum GMesh::getGxBlendMode() { return m_blendMode; }

bool GMesh::getIsTransparent() { return m_isTransparent; }

MeshType GMesh::getMeshType() {
    return m_meshType;
}

void GMesh::setRenderOrder(int renderOrder) {
    m_renderOrder = renderOrder;
}

void GMesh::setEnd(int end) {m_end = end; }


