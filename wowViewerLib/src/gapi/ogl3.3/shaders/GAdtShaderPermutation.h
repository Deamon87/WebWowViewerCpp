//
// Created by Deamon on 7/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_GADTSHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_GADTSHADERPERMUTATION_H

#include "../GShaderPermutation.h"

class GAdtShaderPermutation : public GShaderPermutation{
    friend class GDeviceGL33;
protected:
    explicit GAdtShaderPermutation(std::string &shaderName, IDevice *device);

    virtual void compileShader() override;

};


#endif //AWEBWOWVIEWERCPP_GADTSHADERPERMUTATION_H
