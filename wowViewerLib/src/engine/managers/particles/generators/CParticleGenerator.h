//
// Created by deamon on 15.12.17.
//

#ifndef WEBWOWVIEWERCPP_CPARTICLEGENERATOR_H
#define WEBWOWVIEWERCPP_CPARTICLEGENERATOR_H


#include <mathfu/glsl_mappings.h>
#include "../particle.h"
#include "../CGeneratorAniProp.h"
#include "../CRndSeed.h"

class CParticleGenerator {
    CParticleGenerator() {
        seed = new CRndSeed(0);
    }

public:
    virtual float GetEmissionRate();
    virtual void Update(float time, mathfu::mat4 &viewMat);
    virtual void CreateParticle(CParticle2 &, float);
private:
    CGeneratorAniProp aniProp;
    CRndSeed *seed;

    float GetMaxEmissionRate();

    float GetMaxLifeSpan();

    float GetLifeSpan(float m);

    void CalcVelocity(CParticle2 *p, float delta);
};


#endif //WEBWOWVIEWERCPP_CPARTICLEGENERATOR_H
