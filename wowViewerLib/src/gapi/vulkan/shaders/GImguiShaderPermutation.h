//
// Created by Deamon on 26.03.2020.
//

#ifndef AWEBWOWVIEWERCPP_GIMGUISHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_GIMGUISHADERPERMUTATION_H

#include "GShaderPermutationVLK.h"

class GImguiShaderPermutation : public GShaderPermutationVLK {
    friend class GDeviceVLK;
public:
    ~GImguiShaderPermutation() override {};

protected:
    explicit GImguiShaderPermutation(std::string &shaderName, IDevice *device);
public:
    int getTextureBindingStart() override {
        return 5;
    };
    int getTextureCount() override {
        return 1;
    };
};


#endif //AWEBWOWVIEWERCPP_GIMGUISHADERPERMUTATION_H
