//
// Created by deamon on 09.08.18.
//

#ifndef AWEBWOWVIEWERCPP_GOCCLUSIONQUERY_H
#define AWEBWOWVIEWERCPP_GOCCLUSIONQUERY_H

#include "meshes/GMesh.h"

class GOcclusionQuery : public GMesh {
    friend class GDevice;
public:
    ~GOcclusionQuery() override;
protected:
    GOcclusionQuery(GDevice &device, HGMesh &oclludee);


private:


    void createQuery();;
    void deleteQuery();;

    void beginQuery();
    void endQuery();

    void beginConditionalRendering();
    void endConditionalRendering();


    int getResult();

private:
    GLuint m_queryIdent;
};


#endif //AWEBWOWVIEWERCPP_GOCCLUSIONQUERY_H
