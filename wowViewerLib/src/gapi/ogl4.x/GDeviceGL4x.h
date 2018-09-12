//
// Created by Deamon on 9/3/2018.
//

#ifndef AWEBWOWVIEWERCPP_GDEVICEGL4X_H
#define AWEBWOWVIEWERCPP_GDEVICEGL4X_H

class GUniformBufferGL4x;
class GMeshGL4x;


#include <memory>
#include "buffers/GUniformBufferGL4x.h"
#include "meshes/GMeshGL4x.h"
#include "meshes/GParticleMesh4x.h"
#include "meshes/GM2MeshGL4x.h"

#include "../ogl3.3/GDevice.h"


typedef std::shared_ptr<GUniformBufferGL4x> HGLUniformBufferGL4x;
typedef std::shared_ptr<GMeshGL4x> HGLMeshGL4x;


class GDeviceGL4x : public GDeviceGL33 {
public:
    void drawMeshes(std::vector<HGMesh> &meshes) override;
    void updateBuffers(std::vector<HGMesh> &iMeshes);
    void drawMesh(HGMesh &hIMesh);

    HGMesh createMesh(gMeshTemplate &meshTemplate) override;
    HGM2Mesh createM2Mesh(gMeshTemplate &meshTemplate) override;
};


#endif //AWEBWOWVIEWERCPP_GDEVICEGL4X_H
