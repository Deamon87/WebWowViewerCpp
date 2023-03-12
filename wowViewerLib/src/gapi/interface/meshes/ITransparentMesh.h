//
// Created by Deamon on 19.12.22.
//

#ifndef AWEBWOWVIEWERCPP_ITRANSPARENTMESH_H
#define AWEBWOWVIEWERCPP_ITRANSPARENTMESH_H

#include "IMesh.h"

class ITransparentMesh : virtual public IMesh {
    friend class IDevice;
protected:
    float m_sortDistance = 0;
public:
    ~ITransparentMesh() override = default;

    auto sortDistance()       -> float { return m_sortDistance; }

    virtual void setSortDistance(float distance) = 0;
    virtual float getSortDistance() = 0;
};
#endif //AWEBWOWVIEWERCPP_ITRANSPARENTMESH_H
