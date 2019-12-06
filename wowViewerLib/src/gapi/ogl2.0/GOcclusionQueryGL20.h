//
// Created by deamon on 09.08.18.
//

#ifndef AWEBWOWVIEWERCPP_GOCCLUSIONQUERY_H
#define AWEBWOWVIEWERCPP_GOCCLUSIONQUERY_H

#include "meshes/GMeshGL20.h"

class GOcclusionQueryGL20 : public GMeshGL20 {
    friend class GDeviceGL20;
public:
    ~GOcclusionQueryGL20() override;
protected:
    GOcclusionQueryGL20(IDevice &device, HGMesh &oclludee);


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
