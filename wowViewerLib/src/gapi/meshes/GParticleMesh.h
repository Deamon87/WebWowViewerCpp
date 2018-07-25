//
// Created by Deamon on 7/24/2018.
//

#ifndef AWEBWOWVIEWERCPP_GPARTICLEMESH_H
#define AWEBWOWVIEWERCPP_GPARTICLEMESH_H
#include "GMesh.h"

class GParticleMesh : public GMesh {
    friend class GDevice;
protected:
    GParticleMesh(GDevice &device, const gMeshTemplate &meshTemplate);
};


#endif //AWEBWOWVIEWERCPP_GPARTICLEMESH_H
