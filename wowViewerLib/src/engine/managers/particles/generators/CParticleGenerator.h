//
// Created by deamon on 15.12.17.
//

#ifndef WEBWOWVIEWERCPP_CPARTICLEGENERATOR_H
#define WEBWOWVIEWERCPP_CPARTICLEGENERATOR_H


#include <mathfu/glsl_mappings.h>
#include "../particle.h"
#include "../CGeneratorAniProp.h"
#include "../CRndSeed.h"
#include "../../../persistance/header/M2FileHeader.h"

class CParticleGenerator {
public:
    CParticleGenerator(CRndSeed &seed, M2Particle *particle) {
        this->seed = seed;
        this->m_particleData = particle;
    }

public:
    virtual float GetEmissionRate();
    virtual void Update(animTime_t time, mathfu::mat4 &viewMat);
    virtual void CreateParticle(CParticle2 &, animTime_t ) = 0;

protected:
    CGeneratorAniProp aniProp;
    CRndSeed seed = CRndSeed(0);
    M2Particle *m_particleData;

public:
    CGeneratorAniProp *getAniProp(){ return &aniProp;};
    float GetSpeedVariation();
    float GetMaxEmissionRate();

    float GetMaxLifeSpan();

    float GetLifeSpan(float m);
    mathfu::vec3 GetGravity();

    float CalcVelocity( );
};


#endif //WEBWOWVIEWERCPP_CPARTICLEGENERATOR_H
