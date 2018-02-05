//
// Created by deamon on 15.12.17.
//

#ifndef WEBWOWVIEWERCPP_CSPHEREGENERATOR_H
#define WEBWOWVIEWERCPP_CSPHEREGENERATOR_H


#include "CParticleGenerator.h"

class CSphereGenerator : public CParticleGenerator {
public:
    CSphereGenerator(CRndSeed &seed, bool particlesGoUp) : CParticleGenerator(seed) {
        ;
    }
};


#endif //WEBWOWVIEWERCPP_CSPHEREGENERATOR_H
