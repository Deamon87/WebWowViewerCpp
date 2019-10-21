//
// Created by Deamon on 7/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_GWMOSHADERPERMUTATION_4X_H
#define AWEBWOWVIEWERCPP_GWMOSHADERPERMUTATION_4X_H

#include "../GShaderPermutationGL4x.h"

class GWMOShaderPermutationGL4x : public GShaderPermutationGL4x {
    friend class GDeviceGL4x;
protected:
    explicit GWMOShaderPermutationGL4x(std::string &shaderName, IDevice *device);

    virtual void compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) override;

};


#endif //AWEBWOWVIEWERCPP_GWMOSHADERPERMUTATION_4X_H
