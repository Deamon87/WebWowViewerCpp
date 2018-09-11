//
// Created by Deamon on 7/24/2018.
//

#ifndef AWEBWOWVIEWERCPP_GPARTICLEMESH_GL4x_H
#define AWEBWOWVIEWERCPP_GPARTICLEMESH_GL4x_H

#include "GMeshGL4x.h"

class GParticleMesh4x : public GMeshGL4x {
    friend class GDeviceGL4x;
protected:
    GParticleMesh4x(IDevice &device, const gMeshTemplate &meshTemplate);
};


#endif //AWEBWOWVIEWERCPP_GPARTICLEMESH_GL4x_H
