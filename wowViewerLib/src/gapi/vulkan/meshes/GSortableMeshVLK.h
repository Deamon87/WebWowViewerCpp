//
// Created by Deamon on 3/20/2023.
//

#ifndef AWEBWOWVIEWERCPP_GSORTABLEMESHVLK_H
#define AWEBWOWVIEWERCPP_GSORTABLEMESHVLK_H


#include "GMeshVLK.h"
#include "../../interface/meshes/ISortableMesh.h"

class GSortableMeshVLK : public GMeshVLK, public virtual ISortableMesh {
public:
    GSortableMeshVLK(const gMeshTemplate &meshTemplate, const HMaterialVLK &material,
                     int priorityPlane);

    ~GSortableMeshVLK() override = default;

    EGxBlendEnum getGxBlendMode() override {
        return material()->getBlendMode();
    }
};


#endif //AWEBWOWVIEWERCPP_GSORTABLEMESHVLK_H
