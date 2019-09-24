//
// Created by Deamon on 7/8/2018.
//

#ifndef AWEBWOWVIEWERCPP_GAdtSHADERPERMUTATIONVLK_H
#define AWEBWOWVIEWERCPP_GAdtSHADERPERMUTATIONVLK_H

#include "GShaderPermutationVLK.h"

class GAdtShaderPermutationVLK : public GShaderPermutationVLK {
    friend class GDeviceVLK;
public:
    ~GAdtShaderPermutationVLK() override {};

protected:
    explicit GAdtShaderPermutationVLK(std::string &shaderName, IDevice *device);
public:
    int getTextureBindingStart() override {
        return 5;
    };
    int getTextureCount() override {
        return 9;
    };
};


#endif //AWEBWOWVIEWERCPP_GM2PARTICLESHADERPERMUTATION_H
