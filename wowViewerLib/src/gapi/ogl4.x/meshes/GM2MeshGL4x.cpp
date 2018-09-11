//
// Created by Deamon on 7/22/2018.
//

#include "GM2MeshGL4x.h"

GM2MeshGL4x::GM2MeshGL4x(IDevice &device, const gMeshTemplate &meshTemplate) : GMeshGL4x(device, meshTemplate){
    m_meshType = MeshType::eM2Mesh;
}

void GM2MeshGL4x::setM2Object(void *m2Object) {
    m_m2Object = (m2Object);
}

void GM2MeshGL4x::setLayer(int layer) {
    m_layer = layer;
}

void GM2MeshGL4x::setPriorityPlane(int priorityPlane) {
    m_priorityPlane = priorityPlane;
}

void GM2MeshGL4x::setSortDistance(float distance) {
    m_sortDistance = distance;
}

void GM2MeshGL4x::setQuery(const HGOcclusionQuery &query) {
    m_query = query;
}
