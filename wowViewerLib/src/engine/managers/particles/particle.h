//
// Created by deamon on 15.12.17.
//

#ifndef WEBWOWVIEWERCPP_PARTICLE_H
#define WEBWOWVIEWERCPP_PARTICLE_H


#include "../../persistance/header/commonFileStructs.h"
#include "../../../include/iostuff.h"

class CParticle2 {
public:
    mathfu::vec3 position = {0,0,0};
    animTime_t age = 0.0f;
    mathfu::vec3 velocity = {0,0,0};
    int16_t state;
    uint16_t seed = 0;
    mathfu::vec2 texPos[2] = {{0,0}, {0,0}};
    mathfu::vec2 texVel[2] = {{0,0}, {0,0}};
    bool isDead = false;
};


#endif //WEBWOWVIEWERCPP_PARTICLE_H
