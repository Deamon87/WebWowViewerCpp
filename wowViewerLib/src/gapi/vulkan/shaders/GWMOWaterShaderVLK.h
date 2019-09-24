//
// Created by Deamon on 7/8/2018.
//

#ifndef AWEBWOWVIEWERCPP_GWMOWATERSHADERPERMUTATIONVLK_H
#define AWEBWOWVIEWERCPP_GWMOWATERSHADERPERMUTATIONVLK_H

#include "GShaderPermutationVLK.h"

class GWmoWaterShaderPermutationVLK : public GShaderPermutationVLK {
    friend class GDeviceVLK;
public:
    ~GWmoWaterShaderPermutationVLK() override {};

protected:
    explicit GWmoWaterShaderPermutationVLK(std::string &shaderName, IDevice *device);
public:
    int getTextureBindingStart() override {
        return 0;
    };
    int getTextureCount() override {
        return 0;
    };
};


#endif //AWEBWOWVIEWERCPP_GWMOWATERSHADERPERMUTATIONVLK_H
