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
public:
    CParticleGenerator(CRndSeed *seed) {
        this->seed = seed;
    }

public:
    virtual float GetEmissionRate();
    virtual void Update(animTime_t time, mathfu::mat4 &viewMat);
    virtual void CreateParticle(CParticle2 &, animTime_t );
private:
    CGeneratorAniProp aniProp;
    CRndSeed *seed;

public:
    CGeneratorAniProp &getAniProp(){ return aniProp;};
    float GetSpeedVariation();
    float GetMaxEmissionRate();

    float GetMaxLifeSpan();

    float GetLifeSpan(float m);
    mathfu::vec3 GetGravity();

    void CalcVelocity(CParticle2 *p, animTime_t  delta);
};


#endif //WEBWOWVIEWERCPP_CPARTICLEGENERATOR_H
