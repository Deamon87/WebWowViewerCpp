//
// Created by deamon on 09.08.18.
//

#include "GOcclusionQuery.h"


GOcclusionQuery::GOcclusionQuery(GDevice &device, HGMesh &oclludee) : GMesh(*oclludee){
    createQuery();

    m_meshType = MeshType::eOccludingQuery;
}

GOcclusionQuery::~GOcclusionQuery() {
    deleteQuery();
}

void GOcclusionQuery::createQuery() {
    glGenQueries(1, &m_queryIdent);
}

void GOcclusionQuery::deleteQuery() {
    glDeleteQueries(1, &m_queryIdent);
}

void GOcclusionQuery::beginQuery() {
    glBeginQuery(GL_SAMPLES_PASSED, m_queryIdent);
}

void GOcclusionQuery::endQuery() {
    glEndQuery(GL_SAMPLES_PASSED);
}

void GOcclusionQuery::beginConditionalRendering() {
    glBeginConditionalRender(m_queryIdent, GL_QUERY_NO_WAIT);
}

void GOcclusionQuery::endConditionalRendering() {
    glEndConditionalRender();
}

int GOcclusionQuery::getResult() {
    int ns;
    glGetQueryObjectiv(m_queryIdent, GL_QUERY_RESULT, &ns);
    return ns;
}
