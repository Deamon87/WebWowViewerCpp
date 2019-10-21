//
// Created by Deamon on 7/8/2018.
//

#ifndef AWEBWOWVIEWERCPP_GM2RIBBONSHADERPERMUTATIONVLK_H
#define AWEBWOWVIEWERCPP_GM2RIBBONSHADERPERMUTATIONVLK_H

#include "GShaderPermutationVLK.h"

class GM2RibbonShaderPermutationVLK : public GShaderPermutationVLK {
    friend class GDeviceVLK;
public:
    ~GM2RibbonShaderPermutationVLK() override {};

protected:
    explicit GM2RibbonShaderPermutationVLK(std::string &shaderName, IDevice *device);
public:
    int getTextureBindingStart() override {
        return 5;
    };
    int getTextureCount() override {
        return 3;
    };
};


#endif //AWEBWOWVIEWERCPP_GM2RIBBONSHADERPERMUTATIONVLK_H
