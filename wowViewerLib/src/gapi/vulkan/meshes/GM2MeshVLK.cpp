//
// Created by Deamon on 7/22/2018.
//

#include "GM2MeshVLK.h"

GM2MeshVLK::GM2MeshVLK(IDevice &device, const gMeshTemplate &meshTemplate) : GMeshVLK(device, meshTemplate){
//    m_meshType = MeshType::eM2Mesh;
}

void GM2MeshVLK::setM2Object(void *m2Object) {
    m_m2Object = (m2Object);
}

void GM2MeshVLK::setLayer(int layer) {
    m_layer = layer;
}

void GM2MeshVLK::setPriorityPlane(int priorityPlane) {
    m_priorityPlane = priorityPlane;
}

void GM2MeshVLK::setSortDistance(float distance) {
    m_sortDistance = distance;
}
float GM2MeshVLK::getSortDistance() {
    return m_sortDistance;
}

void GM2MeshVLK::setQuery(const HGOcclusionQuery &query) {
    m_query = query;
}
