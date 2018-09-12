//
// Created by deamon on 09.08.18.
//

#ifndef AWEBWOWVIEWERCPP_GOCCLUSIONQUERY_4X_H
#define AWEBWOWVIEWERCPP_GOCCLUSIONQUERY_4X_H

#include "meshes/GMeshGL4x.h"

class GOcclusionQueryGL4x : public GMeshGL4x {
    friend class GDeviceGL4x;
public:
    ~GOcclusionQueryGL4x() override;
protected:
    GOcclusionQueryGL4x(IDevice &device, HGMesh &oclludee);


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


#endif //AWEBWOWVIEWERCPP_GOCCLUSIONQUERY_4X_H
