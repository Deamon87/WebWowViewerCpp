//
// Created by deamon on 09.08.18.
//

#include "GOcclusionQueryGL20.h"


GOcclusionQueryGL20::GOcclusionQueryGL20(IDevice &device, HGMesh &oclludee) : GMeshGL20(*(GMeshGL20 *)(oclludee.get())){
    createQuery();

    m_meshType = MeshType::eOccludingQuery;
}

GOcclusionQueryGL20::~GOcclusionQueryGL20() {
    deleteQuery();
}

void GOcclusionQueryGL20::createQuery() {
    glGenQueries(1, &m_queryIdent);
}

void GOcclusionQueryGL20::deleteQuery() {
    glDeleteQueries(1, &m_queryIdent);
}

void GOcclusionQueryGL20::beginQuery() {
    glBeginQuery(GL_SAMPLES_PASSED, m_queryIdent);
}

void GOcclusionQueryGL20::endQuery() {
    glEndQuery(GL_SAMPLES_PASSED);
}

void GOcclusionQueryGL20::beginConditionalRendering() {
//    glBeginConditionalRender(m_queryIdent, GL_QUERY_NO_WAIT);
}

void GOcclusionQueryGL20::endConditionalRendering() {
//    glEndConditionalRender();
}

int GOcclusionQueryGL20::getResult() {
    int ns;
    glGetQueryObjectiv(m_queryIdent, GL_QUERY_RESULT, &ns);
    return ns;
}
