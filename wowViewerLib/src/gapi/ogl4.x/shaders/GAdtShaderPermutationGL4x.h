//
// Created by Deamon on 7/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_GADTSHADERPERMUTATION_4X_H
#define AWEBWOWVIEWERCPP_GADTSHADERPERMUTATION_4X_H

#include "../GShaderPermutationGL4x.h"

class GAdtShaderPermutationGL4x : public GShaderPermutationGL4x {
    friend class GDeviceGL4x;
protected:
    explicit GAdtShaderPermutationGL4x(std::string &shaderName, IDevice *device);

    void compileShader() override;

};


#endif //AWEBWOWVIEWERCPP_GADTSHADERPERMUTATION_4X_H
