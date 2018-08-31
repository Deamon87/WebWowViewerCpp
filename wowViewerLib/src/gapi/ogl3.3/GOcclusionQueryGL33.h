//
// Created by deamon on 09.08.18.
//

#ifndef AWEBWOWVIEWERCPP_GOCCLUSIONQUERY_H
#define AWEBWOWVIEWERCPP_GOCCLUSIONQUERY_H

#include "meshes/GMeshGL33.h"

class GOcclusionQueryGL33 : public GMeshGL33, public virtual IOcclusionQuery {
    friend class GDevice;
public:
    ~GOcclusionQueryGL33() override;
protected:
    GOcclusionQueryGL33(GDevice &device, HGMesh &oclludee);


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
