//
// Created by Deamon on 7/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_GADTSHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_GADTSHADERPERMUTATION_H

#include "../GShaderPermutationGL33.h"

class GAdtShaderPermutationGL33 : public GShaderPermutationGL33 {
    friend class GDeviceGL33;
protected:
    explicit GAdtShaderPermutationGL33(std::string &shaderName, IDevice *device);

    void compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) override;

};


#endif //AWEBWOWVIEWERCPP_GADTSHADERPERMUTATION_H
