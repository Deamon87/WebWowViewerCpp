//
// Created by Deamon on 7/8/2018.
//

#ifndef AWEBWOWVIEWERCPP_GM2SHADERPERMUTATION_4X_H
#define AWEBWOWVIEWERCPP_GM2SHADERPERMUTATION_4X_H

#include "../GShaderPermutationGL4x.h"

class GM2ShaderPermutationGL4x : public GShaderPermutationGL4x{
    friend class GDeviceGL4x;
public:
    ~GM2ShaderPermutationGL4x() override {};

protected:
    explicit GM2ShaderPermutationGL4x(std::string &shaderName, IDevice *device);

    void compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) override;

};


#endif //AWEBWOWVIEWERCPP_GM2SHADERPERMUTATION_4X_H
