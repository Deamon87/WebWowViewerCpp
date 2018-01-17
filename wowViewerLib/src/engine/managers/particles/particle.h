//
// Created by deamon on 15.12.17.
//

#ifndef WEBWOWVIEWERCPP_PARTICLE_H
#define WEBWOWVIEWERCPP_PARTICLE_H


#include "../../persistance/header/commonFileStructs.h"
#include "../../../include/wowScene.h"

class CParticle2 {
    C3Vector position;
    animTime_t age;
    C3Vector velocity;
    fixed16 lifespan;
    uint16_t seed;
    C3Vector texPos;
    float texVel;
};


#endif //WEBWOWVIEWERCPP_PARTICLE_H
