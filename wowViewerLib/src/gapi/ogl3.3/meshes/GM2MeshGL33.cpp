//
// Created by Deamon on 7/22/2018.
//

#include "GM2MeshGL33.h"

GM2MeshGL33::GM2MeshGL33(IDevice &device, const gMeshTemplate &meshTemplate) : GMeshGL33(device, meshTemplate){
    m_meshType = MeshType::eM2Mesh;
}

void GM2MeshGL33::setM2Object(void *m2Object) {
    m_m2Object = (m2Object);
}

void GM2MeshGL33::setLayer(int layer) {
    m_layer = layer;
}

void GM2MeshGL33::setPriorityPlane(int priorityPlane) {
    m_priorityPlane = priorityPlane;
}

void GM2MeshGL33::setSortDistance(float distance) {
    m_sortDistance = distance;
}

void GM2MeshGL33::setQuery(const HGOcclusionQuery &query) {
    m_query = query;
}
