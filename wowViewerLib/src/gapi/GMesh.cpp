//
// Created by deamon on 05.06.18.
//

#include "GMesh.h"

GMesh::GMesh(GDevice &device,
             gMeshTemplate meshTemplate
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

    m_uniformBuffer[0] = meshTemplate.buffers[0];
    m_uniformBuffer[1] = meshTemplate.buffers[1];
    m_uniformBuffer[2] = meshTemplate.buffers[2];

}
GMesh::~GMesh() {

}


