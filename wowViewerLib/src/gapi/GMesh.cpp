//
// Created by deamon on 05.06.18.
//

#include "GMesh.h"

GMesh::GMesh(GDevice &device,
             const gMeshTemplate &meshTemplate
) : m_device(device), m_bindings(meshTemplate.bindings), m_shader(meshTemplate.shader) {

    m_depthWrite = meshTemplate.depthWrite;
    m_depthCulling = meshTemplate.depthCulling;
    m_backFaceCulling = meshTemplate.backFaceCulling;
    m_blendMode = meshTemplate.blendMode;

    m_start = meshTemplate.start;
    m_end = meshTemplate.end;
    m_element = meshTemplate.element;
    m_textureCount = meshTemplate.textureCount;
    m_texture[0] = meshTemplate.texture[0];
    m_texture[1] = meshTemplate.texture[1];
    m_texture[2] = meshTemplate.texture[2];
    m_texture[3] = meshTemplate.texture[3];

    m_vertexUniformBuffer[0] = meshTemplate.vertexBuffers[0];
    m_vertexUniformBuffer[1] = meshTemplate.vertexBuffers[1];
    m_vertexUniformBuffer[2] = meshTemplate.vertexBuffers[2];

    m_fragmentUniformBuffer[0] = meshTemplate.fragmentBuffers[0];
    m_fragmentUniformBuffer[1] = meshTemplate.fragmentBuffers[1];
    m_fragmentUniformBuffer[2] = meshTemplate.fragmentBuffers[2];

}
GMesh::~GMesh() {

}


