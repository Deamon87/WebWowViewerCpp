//
// Created by Deamon on 7/24/2018.
//

#include "GParticleMeshGL33.h"

GParticleMeshGL33::GParticleMeshGL33(IDevice &device, const gMeshTemplate &meshTemplate) : GMeshGL33(device, meshTemplate){
    m_meshType = MeshType::eParticleMesh;
}
