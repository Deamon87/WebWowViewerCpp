//
// Created by Deamon on 7/22/2018.
//

#ifndef AWEBWOWVIEWERCPP_GM2MESH_H
#define AWEBWOWVIEWERCPP_GM2MESH_H

#include "../../interface/meshes/IM2Mesh.h"
#include "GSortableMeshVLK.h"

class GM2MeshVLK : public GSortableMeshVLK, public IM2Mesh {
public:
    GM2MeshVLK(const gMeshTemplate &meshTemplate, const HMaterialVLK &material, int priorityPlane, int layer);
    ~GM2MeshVLK() override = default ;
};

#endif //AWEBWOWVIEWERCPP_GM2MESH_H
