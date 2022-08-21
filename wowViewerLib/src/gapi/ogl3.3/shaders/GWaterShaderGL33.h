//
// Created by Deamon on 9/13/2020.
//

#ifndef AWEBWOWVIEWERCPP_GWATERSHADER33_H
#define AWEBWOWVIEWERCPP_GWATERSHADER33_H

#include "../GShaderPermutationGL33.h"

class GWaterShaderGL33 : public GShaderPermutationGL33 {
    friend class GDeviceGL33;
protected:
    explicit GWaterShaderGL33(std::string &shaderName, const HGDevice &device);

    virtual void compileShader(const std::string &vertExtraDefStrings, const std::string &fragExtraDefStrings) override;
};


#endif //AWEBWOWVIEWERCPP_GWATERSHADER33_H
