//
// Created by Deamon on 7/24/2018.
//

#ifndef AWEBWOWVIEWERCPP_GPARTICLEMESH_H
#define AWEBWOWVIEWERCPP_GPARTICLEMESH_H
#include "GMeshGL33.h"
#include "../../interface/meshes/IParticleMesh.h"

class GParticleMesh : public GMeshGL33, public virtual IParticleMesh {
    friend class GDeviceGLL33;
protected:
    GParticleMesh(IDevice &device, const gMeshTemplate &meshTemplate);
};


#endif //AWEBWOWVIEWERCPP_GPARTICLEMESH_H
