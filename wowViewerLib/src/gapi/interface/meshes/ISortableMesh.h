//
// Created by Deamon on 19.12.22.
//

#ifndef AWEBWOWVIEWERCPP_ISORTABLEMESH_H
#define AWEBWOWVIEWERCPP_ISORTABLEMESH_H

#include "IMesh.h"

class ISortableMesh : virtual public IMesh {
    friend class IDevice;
protected:
    float m_sortDistance = 0;
    int m_priorityPlane = 0;
public:
    explicit ISortableMesh(int priorityPlane) { m_priorityPlane = priorityPlane; };
    ~ISortableMesh() override = default;

    auto sortDistance()       -> float { return m_sortDistance; }
    auto priorityPlane() const-> int { return m_priorityPlane; }

    void setSortDistance(float distance) { m_sortDistance = distance; };
    float getSortDistance() {    return m_sortDistance;};

    virtual EGxBlendEnum getGxBlendMode() = 0;
};
#endif //AWEBWOWVIEWERCPP_ISORTABLEMESH_H
