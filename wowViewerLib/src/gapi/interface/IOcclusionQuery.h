//
// Created by Deamon on 8/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_OOCCLUSIONQUERY_H
#define AWEBWOWVIEWERCPP_OOCCLUSIONQUERY_H

#include "meshes/IMesh.h"

class IOcclusionQuery : public virtual IMesh {
public:
    virtual ~IOcclusionQuery() = 0;
};

#endif //AWEBWOWVIEWERCPP_OOCCLUSIONQUERY_H
