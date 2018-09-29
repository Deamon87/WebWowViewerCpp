//
// Created by Deamon on 7/8/2018.
//

#ifndef AWEBWOWVIEWERCPP_GM2SHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_GM2SHADERPERMUTATION_H

#include "../GShaderPermutationGL33.h"

class GM2ShaderPermutationGL33 : public GShaderPermutationGL33{
    friend class GDeviceGL33;
public:
    ~GM2ShaderPermutationGL33() override {};

protected:
    explicit GM2ShaderPermutationGL33(std::string &shaderName, IDevice *device, M2ShaderCacheRecord &permutation);

    void compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) override;
    M2ShaderCacheRecord &permutation;
};


#endif //AWEBWOWVIEWERCPP_GM2SHADERPERMUTATION_H
