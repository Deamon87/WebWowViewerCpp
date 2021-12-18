//
// Created by Deamon on 17.05.2020.
//

#ifndef AWEBWOWVIEWERCPP_GSKYCONUS_H
#define AWEBWOWVIEWERCPP_GSKYCONUS_H

#include "../GShaderPermutationGL33.h"

class GSkyConus : public GShaderPermutationGL33 {
    friend class GDeviceGL33;
protected:
    explicit GSkyConus(std::string &shaderName, const HGDevice &device);

    void compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) override;

};


#endif //AWEBWOWVIEWERCPP_GSKYCONUS_H
