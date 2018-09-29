//
// Created by Deamon on 7/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_GWMOSHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_GWMOSHADERPERMUTATION_H

#include "../GShaderPermutationGL33.h"

class GWMOShaderPermutationGL33 : public GShaderPermutationGL33 {
    friend class GDeviceGL33;
protected:
    explicit GWMOShaderPermutationGL33(std::string &shaderName, IDevice *device);

    virtual void compileShader(const std::string &vertExtraDefStrings, const std::string &fragExtraDefStrings) override;

};


#endif //AWEBWOWVIEWERCPP_GWMOSHADERPERMUTATION_H
