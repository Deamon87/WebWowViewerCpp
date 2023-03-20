//
// Created by Deamon on 3/20/2023.
//

#include "GSortableMeshVLK.h"

GSortableMeshVLK::GSortableMeshVLK(const gMeshTemplate &meshTemplate, const HMaterialVLK &material,
                                   int priorityPlane) : GMeshVLK(meshTemplate, material), ISortableMesh(priorityPlane) {

}
