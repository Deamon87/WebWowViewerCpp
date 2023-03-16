//
// Created by Deamon on 8/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_IM2MESH_H
#define AWEBWOWVIEWERCPP_IM2MESH_H

#include "ITransparentMesh.h"

class IM2Mesh : public ITransparentMesh {
protected:
    int m_priorityPlane = 0;
    int m_layer = 0;
public:
    ~IM2Mesh() override = default;

    auto priorityPlane()       -> int { return m_priorityPlane; }
    auto layer()       -> int& { return m_layer; }

    virtual void setPriorityPlane(int priorityPlane) = 0;
    virtual void setLayer(int layer)  = 0;
};

#endif //AWEBWOWVIEWERCPP_IM2MESH_H
