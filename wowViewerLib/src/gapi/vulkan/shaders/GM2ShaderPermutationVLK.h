//
// Created by Deamon on 7/8/2018.
//

#ifndef AWEBWOWVIEWERCPP_GM2SHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_GM2SHADERPERMUTATION_H

#include "GShaderPermutationVLK.h"

class GM2ShaderPermutationVLK : public GShaderPermutationVLK {
    friend class GDeviceVLK;
public:
    ~GM2ShaderPermutationVLK() override {};

protected:
    explicit GM2ShaderPermutationVLK(std::string &shaderName, IDevice *device, M2ShaderCacheRecord &permutation);

    void compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) override;
    M2ShaderCacheRecord &permutation;
};


#endif //AWEBWOWVIEWERCPP_GM2SHADERPERMUTATION_H
