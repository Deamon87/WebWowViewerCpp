//
// Created by Deamon on 12/13/2020.
//

#ifndef AWEBWOWVIEWERCPP_GFFXGAUSS4VLK_H
#define AWEBWOWVIEWERCPP_GFFXGAUSS4VLK_H

#include "GShaderPermutationVLK.h"

class GFFXgauss4VLK : public GShaderPermutationVLK {
    friend class GDeviceVLK;
public:
    ~GFFXgauss4VLK() override {};

protected:
    explicit GFFXgauss4VLK(std::string &shaderName, IDevice *device);
public:
    int getTextureBindingStart() override {
        return 5;
    };
    int getTextureCount() override {
        return 1;
    };
};


#endif //AWEBWOWVIEWERCPP_GFFXGAUSS4VLK_H
