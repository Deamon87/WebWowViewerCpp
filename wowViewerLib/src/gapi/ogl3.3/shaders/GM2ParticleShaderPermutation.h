//
// Created by Deamon on 7/24/2018.
//

#ifndef AWEBWOWVIEWERCPP_CM2PARTICLESHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_CM2PARTICLESHADERPERMUTATION_H

#include "../GShaderPermutation.h"

class GM2ParticleShaderPermutation : public GShaderPermutation {
    friend class GDeviceGL33;
protected:
    explicit GM2ParticleShaderPermutation(std::string &shaderName, IDevice *device);

    virtual void compileShader() override;
};


#endif //AWEBWOWVIEWERCPP_CM2PARTICLESHADERPERMUTATION_H
