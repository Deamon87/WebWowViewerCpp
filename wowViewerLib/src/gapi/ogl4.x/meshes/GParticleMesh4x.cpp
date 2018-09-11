//
// Created by Deamon on 7/24/2018.
//

#include "GParticleMesh4x.h"

GParticleMesh4x::GParticleMesh4x(IDevice &device, const gMeshTemplate &meshTemplate) : GMeshGL4x(device, meshTemplate){
    m_meshType = MeshType::eParticleMesh;
}
