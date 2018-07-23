//
// Created by Deamon on 7/22/2018.
//

#include "GM2Mesh.h"

GM2Mesh::GM2Mesh(GDevice &device, const gMeshTemplate &meshTemplate) : GMesh(device, meshTemplate){
    m_meshType = MeshType::eM2Mesh;
}
