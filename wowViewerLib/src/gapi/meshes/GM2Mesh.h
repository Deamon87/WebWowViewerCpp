//
// Created by Deamon on 7/22/2018.
//

#ifndef AWEBWOWVIEWERCPP_GM2MESH_H
#define AWEBWOWVIEWERCPP_GM2MESH_H

#include "GMesh.h"
#include "../../engine/objects/m2/m2Object.h"

class GM2Mesh : public GMesh {
    friend class GDevice;
protected:
    GM2Mesh(GDevice &device, const gMeshTemplate &meshTemplate);
public:
    float m_sortDistance = 0;
    int m_priorityPlane;
    int m_layer;
    M2Object *m_m2Object = nullptr;

};
#endif //AWEBWOWVIEWERCPP_GM2MESH_H
