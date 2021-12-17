//
// Created by Deamon on 7/24/2018.
//

#ifndef AWEBWOWVIEWERCPP_CM2PARTICLESHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_CM2PARTICLESHADERPERMUTATION_H

#include "../GShaderPermutationGL33.h"

class GM2ParticleShaderPermutationGL33 : public GShaderPermutationGL33 {
    friend class GDeviceGL33;
protected:
    explicit GM2ParticleShaderPermutationGL33(std::string &shaderName, const HGDevice &device);

    virtual void compileShader(const std::string &vertExtraDefStrings, const std::string &fragExtraDefStrings) override;
};


#endif //AWEBWOWVIEWERCPP_CM2PARTICLESHADERPERMUTATION_H
