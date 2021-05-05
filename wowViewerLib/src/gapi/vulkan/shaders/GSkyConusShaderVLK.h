//
// Created by Deamon on 11/28/2020.
//

#ifndef AWEBWOWVIEWERCPP_GSKYCONUSSHADERVLK_H
#define AWEBWOWVIEWERCPP_GSKYCONUSSHADERVLK_H

#include "GShaderPermutationVLK.h"

class GSkyConusShaderVLK : public GShaderPermutationVLK {
    friend class GDeviceVLK;
public:
    ~GSkyConusShaderVLK() override {};

protected:
    explicit GSkyConusShaderVLK(std::string &shaderName, IDevice *device);
public:
    int getTextureBindingStart() override {
        return 0;
    };
    int getTextureCount() override {
        return 0;
    };
};


#endif //AWEBWOWVIEWERCPP_GSKYCONUSSHADERVLK_H
