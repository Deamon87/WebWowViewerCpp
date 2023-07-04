//
// Created by deamon on 05.06.18.
//

#include <iostream>
#include <array>
#include "GMeshVLK.h"
#include "../textures/GTextureVLK.h"
#include "../shaders/GShaderPermutationVLK.h"
#include "../GPipelineVLK.h"

GMeshVLK::GMeshVLK(const gMeshTemplate &meshTemplate,
                   const HMaterialVLK &material
) : m_meshType(meshTemplate.meshType), m_material(material) {

    m_bindings = meshTemplate.bindings;

    m_isScissorsEnabled = meshTemplate.scissorEnabled;
    if (m_isScissorsEnabled) {
        m_scissorSize = meshTemplate.scissorSize;
        assert(m_scissorOffset[0] >= 0 && m_scissorOffset[1] >= 0);
        m_scissorOffset = meshTemplate.scissorOffset;
    }

    m_isTransparent = material->getPipeline()->getIsTransparent();

    m_start = meshTemplate.start;
    m_end = meshTemplate.end;
}

GMeshVLK::~GMeshVLK() {

}

bool GMeshVLK::getIsTransparent() { return m_isTransparent; }

MeshType GMeshVLK::getMeshType() {
    return m_meshType;
}
