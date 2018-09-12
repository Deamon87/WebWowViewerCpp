//
// Created by Deamon on 7/24/2018.
//

#include "GParticleMeshGL4x.h"

GParticleMeshGL4x::GParticleMeshGL4x(IDevice &device, const gMeshTemplate &meshTemplate) : GMeshGL4x(device, meshTemplate){
    m_meshType = MeshType::eParticleMesh;
}
