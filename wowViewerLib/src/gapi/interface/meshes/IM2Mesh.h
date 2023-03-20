//
// Created by Deamon on 8/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_IM2MESH_H
#define AWEBWOWVIEWERCPP_IM2MESH_H

#include "ISortableMesh.h"

class IM2Mesh : virtual public ISortableMesh {
protected:
    int m_layer = 0;
public:
    IM2Mesh(int layer, int priority) : ISortableMesh(priority) { m_layer = layer;}
    ~IM2Mesh() override = default;

    auto layer() const -> int { return m_layer; }
};

#endif //AWEBWOWVIEWERCPP_IM2MESH_H
