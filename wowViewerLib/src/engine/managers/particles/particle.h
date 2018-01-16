//
// Created by deamon on 15.12.17.
//

#ifndef WEBWOWVIEWERCPP_PARTICLE_H
#define WEBWOWVIEWERCPP_PARTICLE_H


#include "../../persistance/header/commonFileStructs.h"

class CParticle2 {
    C3Vector position;
    float age;
    C3Vector velocity;
    fixed16 lifespan;
    uint16_t seed;
};


#endif //WEBWOWVIEWERCPP_PARTICLE_H
