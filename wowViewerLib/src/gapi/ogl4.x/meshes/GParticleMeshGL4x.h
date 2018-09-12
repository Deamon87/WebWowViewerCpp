//
// Created by Deamon on 7/24/2018.
//

#ifndef AWEBWOWVIEWERCPP_GPARTICLEMESH_4X_H
#define AWEBWOWVIEWERCPP_GPARTICLEMESH_4X_H
#include "GMeshGL4x.h"
#include "../../interface/meshes/IParticleMesh.h"

class GParticleMeshGL4x : public GMeshGL4x {
    friend class GDeviceGL4x;
protected:
    GParticleMeshGL4x(IDevice &device, const gMeshTemplate &meshTemplate);
};


#endif //AWEBWOWVIEWERCPP_GPARTICLEMESH_4X_H
