//
// Created by deamon on 15.12.17.
//

#include "CSphereGenerator.h"
#include "../../../algorithms/mathHelper.h"

void CSphereGenerator::CreateParticle(CParticle2 &p, animTime_t delta) {

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
    float emissionArea = this->aniProp.emissionAreaY - this->aniProp.emissionAreaX;
    float radius = this->aniProp.emissionAreaX + emissionArea* this->seed.UniformPos();
    float polar = this->aniProp.verticalRange * this->seed.Uniform();
    float azimuth = this->aniProp.horizontalRange * this->seed.Uniform();
    float cosPolar = cos(polar);
    mathfu::vec3 emissionDirection = mathfu::vec3(
            cosPolar * cos(azimuth),
            cosPolar * sin(azimuth),
            sin(polar)
    );

    p.position = emissionDirection * radius;
    float velocity = this->CalcVelocity();
    float zSource = this->aniProp.zSource;

    mathfu::vec3 r0(0.5,0.5,0.5);
    if (feq(zSource, 0)) {
        if (this->particlesGoUp) {
            r0 = mathfu::vec3(0,0,1);
        } else {
            r0 = mathfu::vec3(cosPolar * cos(azimuth), cosPolar * sin(azimuth), sin(polar));
        }
    }
    else {
        r0 = mathfu::vec3(0, 0, zSource);
        r0 = p.position - r0;
        if (r0.Length() > 0.0001) {
            r0 = r0.Normalized();
        }
    }
    mathfu::vec3 velocityCandidate = r0 * velocity;
    p.velocity = velocityCandidate;


//    mathfu::mat4 rotateMat = MathHelper::RotationY(toRadian(-90));
//
//    p.position = rotateMat * p.position;
//    p.velocity = rotateMat.Transpose().Inverse() * p.velocity;

}
