//
// Created by Deamon on 12/13/2020.
//

#ifndef AWEBWOWVIEWERCPP_GFFXGLOWVLK_H
#define AWEBWOWVIEWERCPP_GFFXGLOWVLK_H

#include "GShaderPermutationVLK.h"

class GFFXGlowVLK : public GShaderPermutationVLK {
    friend class GDeviceVLK;
public:
    ~GFFXGlowVLK() override {};

protected:
    explicit GFFXGlowVLK(std::string &shaderName, IDevice *device);
public:
    int getTextureBindingStart() override {
        return 5;
    };
    int getTextureCount() override {
        return 2;
    };
};


#endif //AWEBWOWVIEWERCPP_GFFXGLOWVLK_H
