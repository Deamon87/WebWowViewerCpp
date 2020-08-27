//
// Created by Deamon on 26.03.2020.
//

#ifndef AWEBWOWVIEWERCPP_GADTWATERSHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_GADTWATERSHADERPERMUTATION_H


#include "GShaderPermutationVLK.h"

class GAdtWaterShaderPermutation: public GShaderPermutationVLK {
    friend class GDeviceVLK;
public:
    ~GAdtWaterShaderPermutation() override {};

protected:
    explicit GAdtWaterShaderPermutation(std::string &shaderName, IDevice *device);
public:
    int getTextureBindingStart() override {
        return 5;
    };
    int getTextureCount() override {
        return 1;
    };
};


#endif //AWEBWOWVIEWERCPP_GADTWATERSHADERPERMUTATION_H
