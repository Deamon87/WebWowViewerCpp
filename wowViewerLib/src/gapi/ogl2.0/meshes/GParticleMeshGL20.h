//
// Created by Deamon on 7/24/2018.
//

#ifndef AWEBWOWVIEWERCPP_GPARTICLEMESH_H
#define AWEBWOWVIEWERCPP_GPARTICLEMESH_H
#include "GMeshGL20.h"
#include "../../interface/meshes/IParticleMesh.h"

class GParticleMeshGL20 : public GMeshGL20 {
    friend class GDeviceGL20;
protected:
    GParticleMeshGL20(IDevice &device, const gMeshTemplate &meshTemplate);
public:
    float getSortDistance() override {
        return m_sortDistance;
    }
    void setSortDistance(float val) override {m_sortDistance = val;}
    void setPriorityPlane(int priorityPlane) override {
        m_priorityPlane = priorityPlane;
    }
};


#endif //AWEBWOWVIEWERCPP_GPARTICLEMESH_H
