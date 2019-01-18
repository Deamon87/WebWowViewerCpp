//
// Created by Deamon on 7/24/2018.
//

#ifndef AWEBWOWVIEWERCPP_GPARTICLEMESH_H
#define AWEBWOWVIEWERCPP_GPARTICLEMESH_H
#include "GMeshGL33.h"
#include "../../interface/meshes/IParticleMesh.h"

class GParticleMeshGL33 : public GMeshGL33 {
    friend class GDeviceGL33;
protected:
    GParticleMeshGL33(IDevice &device, const gMeshTemplate &meshTemplate);
public:
    float getSortDistance() override {
        return m_sortDistance;
    }
    void setSortDistance(float val) override {m_sortDistance = val;}
};


#endif //AWEBWOWVIEWERCPP_GPARTICLEMESH_H
