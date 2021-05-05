//
// Created by Deamon on 1/7/2021.
//

#ifndef AWEBWOWVIEWERCPP_GDRAWBOUNDINGBOXVLK_H
#define AWEBWOWVIEWERCPP_GDRAWBOUNDINGBOXVLK_H

#include "GShaderPermutationVLK.h"

class GDrawBoundingBoxVLK : public GShaderPermutationVLK {
    friend class GDeviceVLK;
public:
    ~GDrawBoundingBoxVLK() override {};

protected:
    explicit GDrawBoundingBoxVLK(std::string &shaderName, IDevice *device);
public:
    int getTextureBindingStart() override {
        return 5;
    };
    int getTextureCount() override {
        return 0;
    };
};

#endif //AWEBWOWVIEWERCPP_GDRAWBOUNDINGBOXVLK_H
