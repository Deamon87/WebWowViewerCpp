//
// Created by Deamon on 7/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_GWMOSHADERPERMUTATIONVLK_H
#define AWEBWOWVIEWERCPP_GWMOSHADERPERMUTATIONVLK_H

#include "GShaderPermutationVLK.h"

class GWMOShaderPermutationVLK : public GShaderPermutationVLK {
    friend class GDeviceVLK;
protected:
    explicit GWMOShaderPermutationVLK(std::string &shaderName, IDevice *device);
public:
    int getTextureBindingStart() override {
        return 5;
    };
    int getTextureCount() override {
        return 9;
    };
};


#endif //AWEBWOWVIEWERCPP_GWMOSHADERPERMUTATION_H
