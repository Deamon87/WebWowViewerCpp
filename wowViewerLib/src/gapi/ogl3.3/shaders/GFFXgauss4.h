//
// Created by Deamon on 17.05.2020.
//

#ifndef AWEBWOWVIEWERCPP_GFFXGAUSS4_H
#define AWEBWOWVIEWERCPP_GFFXGAUSS4_H

#include "../GShaderPermutationGL33.h"

class GFFXgauss4 : public GShaderPermutationGL33 {
    friend class GDeviceGL33;
protected:
    explicit GFFXgauss4(std::string &shaderName, const HGDevice &device);

    void compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) override;

};


#endif //AWEBWOWVIEWERCPP_GFFXGAUSS4_H
