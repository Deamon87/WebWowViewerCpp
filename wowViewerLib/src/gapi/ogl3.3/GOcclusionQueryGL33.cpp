//
// Created by deamon on 09.08.18.
//

#include "GOcclusionQueryGL33.h"


GOcclusionQueryGL33::GOcclusionQueryGL33(const HGDevice &device, HGMesh oclludee) : GMeshGL33(*(GMeshGL33 *)(oclludee.get())){
    createQuery();

    m_meshType = MeshType::eOccludingQuery;
}

GOcclusionQueryGL33::~GOcclusionQueryGL33() {
    deleteQuery();
}

void GOcclusionQueryGL33::createQuery() {
//    glGenQueries(1, &m_queryIdent);
}

void GOcclusionQueryGL33::deleteQuery() {
//    glDeleteQueries(1, &m_queryIdent);
}

void GOcclusionQueryGL33::beginQuery() {
//    glBeginQuery(GL_SAMPLES_PASSED, m_queryIdent);
}

void GOcclusionQueryGL33::endQuery() {
//    glEndQuery(GL_SAMPLES_PASSED);
}

void GOcclusionQueryGL33::beginConditionalRendering() {
//    glBeginConditionalRender(m_queryIdent, GL_QUERY_NO_WAIT);
}

void GOcclusionQueryGL33::endConditionalRendering() {
//    glEndConditionalRender();
}

int GOcclusionQueryGL33::getResult() {
    int ns;
//    glGetQueryObjectiv(m_queryIdent, GL_QUERY_RESULT, &ns);
    return ns;
}
