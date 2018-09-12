//
// Created by deamon on 09.08.18.
//

#include "GOcclusionQueryGL4x.h"


GOcclusionQueryGL4x::GOcclusionQueryGL4x(IDevice &device, HGMesh &oclludee) : GMeshGL4x(*(GMeshGL4x *)(oclludee.get())){
    createQuery();

    m_meshType = MeshType::eOccludingQuery;
}

GOcclusionQueryGL4x::~GOcclusionQueryGL4x() {
    deleteQuery();
}

void GOcclusionQueryGL4x::createQuery() {
    glGenQueries(1, &m_queryIdent);
}

void GOcclusionQueryGL4x::deleteQuery() {
    glDeleteQueries(1, &m_queryIdent);
}

void GOcclusionQueryGL4x::beginQuery() {
    glBeginQuery(GL_SAMPLES_PASSED, m_queryIdent);
}

void GOcclusionQueryGL4x::endQuery() {
    glEndQuery(GL_SAMPLES_PASSED);
}

void GOcclusionQueryGL4x::beginConditionalRendering() {
    glBeginConditionalRender(m_queryIdent, GL_QUERY_NO_WAIT);
}

void GOcclusionQueryGL4x::endConditionalRendering() {
    glEndConditionalRender();
}

int GOcclusionQueryGL4x::getResult() {
    int ns;
    glGetQueryObjectiv(m_queryIdent, GL_QUERY_RESULT, &ns);
    return ns;
}
