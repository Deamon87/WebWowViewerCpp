//
// Created by Deamon on 8/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_IM2MESH_H
#define AWEBWOWVIEWERCPP_IM2MESH_H
#include "./IMesh.h"

class IM2Mesh : public IMesh {
public:
    virtual void setM2Object(void * m2Object) = 0;
    virtual void setLayer(int layer)  = 0;
    virtual void setPriorityPlane(int priorityPlane) = 0;
    virtual void setQuery(const HGOcclusionQuery &query) = 0;
    virtual void setSortDistance(float distance) = 0;
};
#endif //AWEBWOWVIEWERCPP_IM2MESH_H
