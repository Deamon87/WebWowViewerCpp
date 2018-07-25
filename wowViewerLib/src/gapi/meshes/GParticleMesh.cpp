//
// Created by Deamon on 7/24/2018.
//

#include "GParticleMesh.h"

GParticleMesh::GParticleMesh(GDevice &device, const gMeshTemplate &meshTemplate) : GMesh(device, meshTemplate){
    m_meshType = MeshType::eParticleMesh;
}
