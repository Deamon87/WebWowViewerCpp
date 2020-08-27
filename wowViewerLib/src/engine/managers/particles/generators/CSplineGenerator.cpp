//
// Created by deamon on 20.12.17.
//

#include "CSplineGenerator.h"

void CSplineGenerator::CreateParticle(CParticle2 &p, animTime_t delta) {
    float dvary = delta * this->seed.UniformPos();
    float life = this->seed.Uniform();


    int16_t state;
    if ( life < 1.0 )
    {
        if ( life > -1.0 )
            state = trunc((life * 32767.0f) + 0.5f);
        else
            state = -32767;
    }
    else
    {
        state = 32767;
    }
    p.state = state;

    float lifespan = this->GetLifeSpan(state);
    if (lifespan < 0.001) {
        lifespan = 0.001;
    }
//    p.age = 0.0;
    p.age = fmod(dvary, lifespan);
    p.seed = 0xffff & this->seed.uint32t();

    auto areaX = this->aniProp.emissionAreaX;
    if (areaX >= 0.0) {
        areaX = std::min(areaX, 1.0f);
    } else {
        areaX = 0.0;
    }

    auto areaY = this->aniProp.emissionAreaX;
    if (areaY >= 0.0) {
        areaY = std::min(areaY, 1.0f);
    } else {
        areaY = 0.0;
    }

    float emissionArea = areaY - areaX;
    float radius = areaX + emissionArea* this->seed.UniformPos();

}
