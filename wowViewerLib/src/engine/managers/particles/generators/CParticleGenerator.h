//
// Created by deamon on 15.12.17.
//

#ifndef WEBWOWVIEWERCPP_CPARTICLEGENERATOR_H
#define WEBWOWVIEWERCPP_CPARTICLEGENERATOR_H


#include <mathfu/glsl_mappings.h>
#include "../particle.h"

class CParticleGenerator {
    virtual float GetEmissionRate();
    virtual void Update(float time, mathfu::mat4 &viewMat);
    virtual void CreateParticle(CParticle2 &, float);
};


#endif //WEBWOWVIEWERCPP_CPARTICLEGENERATOR_H
