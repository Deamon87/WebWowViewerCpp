//
// Created by Deamon on 7/24/2018.
//

#ifndef AWEBWOWVIEWERCPP_CM2PARTICLESHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_CM2PARTICLESHADERPERMUTATION_H

#include "../GShaderPermutationGL20.h"

class GM2ParticleShaderPermutationGL20 : public GShaderPermutationGL20 {
    friend class GDeviceGL20;
protected:
    explicit GM2ParticleShaderPermutationGL20(std::string &shaderName, IDevice *device);

    virtual void compileShader(const std::string &vertExtraDefStrings, const std::string &fragExtraDefStrings) override;
};


#endif //AWEBWOWVIEWERCPP_CM2PARTICLESHADERPERMUTATION_H
