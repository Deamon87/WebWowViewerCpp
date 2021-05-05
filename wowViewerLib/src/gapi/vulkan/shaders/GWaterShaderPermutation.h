//
// Created by Deamon on 26.03.2020.
//

#ifndef AWEBWOWVIEWERCPP_GWATERSHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_GWATERSHADERPERMUTATION_H


#include "GShaderPermutationVLK.h"

class GWaterShaderPermutation: public GShaderPermutationVLK {
    friend class GDeviceVLK;
public:
    ~GWaterShaderPermutation() override {};

protected:
    explicit GWaterShaderPermutation(std::string &shaderName, IDevice *device);
public:
    int getTextureBindingStart() override {
        return 5;
    };
    int getTextureCount() override {
        return 1;
    };
};


#endif //AWEBWOWVIEWERCPP_GWATERSHADERPERMUTATION_H
