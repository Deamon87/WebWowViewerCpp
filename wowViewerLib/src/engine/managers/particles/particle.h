//
// Created by deamon on 15.12.17.
//

#ifndef WEBWOWVIEWERCPP_PARTICLE_H
#define WEBWOWVIEWERCPP_PARTICLE_H


#include "../../persistance/header/commonFileStructs.h"
#include "../../../include/wowScene.h"

class CParticle2 {
public:
    mathfu::vec3 position;
    animTime_t age;
    mathfu::vec3 velocity;
    int16_t state;
    uint16_t seed;
    mathfu::vec2 texPos[2];
    mathfu::vec2 texVel[2];
};


#endif //WEBWOWVIEWERCPP_PARTICLE_H
