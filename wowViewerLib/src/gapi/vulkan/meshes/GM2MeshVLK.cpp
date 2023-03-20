//
// Created by Deamon on 7/22/2018.
//

#include "GM2MeshVLK.h"
#include "../GPipelineVLK.h"


GM2MeshVLK::GM2MeshVLK(const gMeshTemplate &meshTemplate,
                       const HMaterialVLK &material, int priorityPlane, int layer) :
                       GSortableMeshVLK(meshTemplate, material, priorityPlane), IM2Mesh(layer, priorityPlane),
                       ISortableMesh(priorityPlane) {

    m_layer = layer;
}
