//
// Created by Deamon on 7/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_GWMOSHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_GWMOSHADERPERMUTATION_H

#include "../GShaderPermutation.h"

class GWMOShaderPermutation : public GShaderPermutation{
    friend class GDevice;
protected:
    explicit GWMOShaderPermutation(std::string &shaderName, GDevice &device);

    virtual void compileShader() override;

};


#endif //AWEBWOWVIEWERCPP_GWMOSHADERPERMUTATION_H
