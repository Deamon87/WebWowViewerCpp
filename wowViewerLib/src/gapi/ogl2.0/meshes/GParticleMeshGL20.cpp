//
// Created by Deamon on 7/24/2018.
//

#include "GParticleMeshGL20.h"

GParticleMeshGL20::GParticleMeshGL20(IDevice &device, const gMeshTemplate &meshTemplate) : GMeshGL20(device, meshTemplate){
    m_meshType = MeshType::eParticleMesh;
}
