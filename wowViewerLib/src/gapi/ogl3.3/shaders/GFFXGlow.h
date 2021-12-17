//
// Created by Deamon on 17.05.2020.
//

#ifndef AWEBWOWVIEWERCPP_GFFXGLOW_H
#define AWEBWOWVIEWERCPP_GFFXGLOW_H

#include "../GShaderPermutationGL33.h"

class GFFXGlow : public GShaderPermutationGL33 {
    friend class GDeviceGL33;
protected:
    explicit GFFXGlow(std::string &shaderName, const HGDevice &device);

    void compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) override;
};


#endif //AWEBWOWVIEWERCPP_GFFXGLOW_H
