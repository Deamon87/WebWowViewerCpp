//
// Created by Deamon on 1/31/2021.
//

#ifndef AWEBWOWVIEWERCPP_GWATERFALLSHADERVLK_H
#define AWEBWOWVIEWERCPP_GWATERFALLSHADERVLK_H

#include "GShaderPermutationVLK.h"

class GWaterfallShaderVLK : public GShaderPermutationVLK{
    friend class GDeviceVLK;
public:
    ~GWaterfallShaderVLK() override {};

protected:
    explicit GWaterfallShaderVLK(std::string &shaderName, IDevice *device);
public:
    int getTextureBindingStart() override {
        return 5;
    };
    int getTextureCount() override {
        return 5;
    };
};


#endif //AWEBWOWVIEWERCPP_GWATERFALLSHADERVLK_H
