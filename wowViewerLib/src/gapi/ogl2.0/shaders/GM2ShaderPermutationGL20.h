//
// Created by Deamon on 7/8/2018.
//

#ifndef AWEBWOWVIEWERCPP_GM2SHADERPERMUTATION20_H
#define AWEBWOWVIEWERCPP_GM2SHADERPERMUTATION20_H

#include "../GShaderPermutationGL20.h"

class GM2ShaderPermutationGL20 : public GShaderPermutationGL20{
    friend class GDeviceGL20;
public:
    ~GM2ShaderPermutationGL20() override {};

protected:
    explicit GM2ShaderPermutationGL20(std::string &shaderName, IDevice *device, M2ShaderCacheRecord &permutation);

    void compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) override;
    M2ShaderCacheRecord &permutation;
};


#endif //AWEBWOWVIEWERCPP_GM2SHADERPERMUTATION_H
