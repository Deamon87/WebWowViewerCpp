//
// Created by deamon on 09.08.18.
//

#ifndef AWEBWOWVIEWERCPP_GOCCLUSIONQUERY_H
#define AWEBWOWVIEWERCPP_GOCCLUSIONQUERY_H


#include "../engine/opengl/header.h"
#include "GDevice.h"

class GOcclusionQuery {
    friend class GDevice;
public:
    GOcclusionQuery(GDevice &device, HGMesh &oclludee) : m_device(device), oclludee(oclludee){
        createQuery();
    }

    ~GOcclusionQuery() {
        deleteQuery();
    }
private:
    void createQuery() {
        glGenQueries(1, &m_queryIdent);
    };
    void deleteQuery() {
        glDeleteQueries(1, &m_queryIdent);
    };

    void beginQuery() {
        glBeginQuery(GL_SAMPLES_PASSED, m_queryIdent);
    }
    void endQuery() {
        glEndQuery(GL_SAMPLES_PASSED);
    }
    int getResult() {
        int ns;
        glGetQueryObjectiv(m_queryIdent, GL_QUERY_RESULT, &ns);
    }

private:

    GDevice &m_device;
    HGMesh &oclludee;
    GLuint m_queryIdent;
};


#endif //AWEBWOWVIEWERCPP_GOCCLUSIONQUERY_H
