//
// Created by deamon on 20.12.17.
//

#ifndef WEBWOWVIEWERCPP_CSPLINEGENERATOR_H
#define WEBWOWVIEWERCPP_CSPLINEGENERATOR_H


#include "CParticleGenerator.h"

class CSplineGenerator : public CParticleGenerator {
public:
    CSplineGenerator(CRndSeed &seed, M2Particle *particle, bool particlesGoUp) : CParticleGenerator(seed, particle), particlesGoUp(particlesGoUp) {

    }
    ~CSplineGenerator() override {} ;

    void CreateParticle(CParticle2 &p, animTime_t delta) override;

private:
    bool particlesGoUp = false;
};

#endif //WEBWOWVIEWERCPP_CSPLINEGENERATOR_H
