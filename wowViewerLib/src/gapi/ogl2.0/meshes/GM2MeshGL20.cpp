//
// Created by Deamon on 7/22/2018.
//

#include "GM2MeshGL20.h"

GM2MeshGL20::GM2MeshGL20(IDevice &device, const gMeshTemplate &meshTemplate) : GMeshGL20(device, meshTemplate){
    m_meshType = MeshType::eM2Mesh;
}

void GM2MeshGL20::setM2Object(void *m2Object) {
    m_m2Object = (m2Object);
}
void *GM2MeshGL20::getM2Object() {
    return m_m2Object;
}

void GM2MeshGL20::setLayer(int layer) {
    m_layer = layer;
}

void GM2MeshGL20::setPriorityPlane(int priorityPlane) {
    m_priorityPlane = priorityPlane;
}

void GM2MeshGL20::setSortDistance(float distance) {
    m_sortDistance = distance;
}
float GM2MeshGL20::getSortDistance() {
    return m_sortDistance;
}

void GM2MeshGL20::setQuery(const HGOcclusionQuery &query) {
    m_query = query;
}


