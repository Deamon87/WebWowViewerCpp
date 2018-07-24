//
// Created by Deamon on 7/24/2018.
//

#ifndef AWEBWOWVIEWERCPP_CM2PARTICLESHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_CM2PARTICLESHADERPERMUTATION_H

#include "../GShaderPermutation.h"

class CM2ParticleShaderPermutation : public GShaderPermutation{
    friend class GDevice;
protected:
    explicit CM2ParticleShaderPermutation(std::string &shaderName, GDevice &device);

    virtual void compileShader() override;
};


#endif //AWEBWOWVIEWERCPP_CM2PARTICLESHADERPERMUTATION_H
