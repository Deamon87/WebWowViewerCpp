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
    explicit GM2ShaderPermutationVLK(std::string &shaderName, IDevice *device);
public:
    int getTextureBindingStart() override {
        return 5;
    };
    int getTextureCount() override {
        return 4;
    };
};


#endif //AWEBWOWVIEWERCPP_GM2SHADERPERMUTATION_H
