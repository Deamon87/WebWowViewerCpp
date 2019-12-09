//
// Created by Deamon on 7/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_GADTSHADERPERMUTATION20_H
#define AWEBWOWVIEWERCPP_GADTSHADERPERMUTATION20_H

#include "../GShaderPermutationGL20.h"

class GAdtShaderPermutationGL20 : public GShaderPermutationGL20 {
    friend class GDeviceGL20;
protected:
    explicit GAdtShaderPermutationGL20(std::string &shaderName, IDevice *device);

    void compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) override;

};


#endif //AWEBWOWVIEWERCPP_GADTSHADERPERMUTATION_H
