//
// Created by deamon on 15.12.17.
//

#ifndef WEBWOWVIEWERCPP_CSPHEREGENERATOR_H
#define WEBWOWVIEWERCPP_CSPHEREGENERATOR_H


#include "CParticleGenerator.h"

class CSphereGenerator : public CParticleGenerator {
public:
    CSphereGenerator(CRndSeed &seed, M2Particle *particle, bool particlesGoUp) : CParticleGenerator(seed, particle), particlesGoUp(particlesGoUp) {

    }

    void CreateParticle(CParticle2 &p, animTime_t delta) override;

private:
    bool particlesGoUp = false;
};


#endif //WEBWOWVIEWERCPP_CSPHEREGENERATOR_H
