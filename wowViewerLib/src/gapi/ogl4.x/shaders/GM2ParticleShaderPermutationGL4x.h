//
// Created by Deamon on 7/24/2018.
//

#ifndef AWEBWOWVIEWERCPP_CM2PARTICLESHADERPERMUTATION_4X_H
#define AWEBWOWVIEWERCPP_CM2PARTICLESHADERPERMUTATION_4X_H

#include "../GShaderPermutationGL4x.h"

class GM2ParticleShaderPermutationGL4x : public GShaderPermutationGL4x {
    friend class GDeviceGL4x;
protected:
    explicit GM2ParticleShaderPermutationGL4x(std::string &shaderName, IDevice *device);

    virtual void compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) override;
};


#endif //AWEBWOWVIEWERCPP_CM2PARTICLESHADERPERMUTATION_4X_H
