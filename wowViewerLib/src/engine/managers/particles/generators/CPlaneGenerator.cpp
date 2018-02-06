//
// Created by deamon on 17.01.18.
//

#include "CPlaneGenerator.h"

void CPlaneGenerator::CreateParticle(CParticle2 &p, animTime_t delta) {

    float dvary = delta * this->seed.UniformPos();
    float life = this->seed.Uniform();
    p.lifespan = life;
    float lifespan = this->GetLifeSpan(life);
    if (lifespan < 0.001) {
        lifespan = 0.001;
    }
    p.age = 0.0;
//    p.age = fmod(dvary, lifespan);
    p.seed = 0xffff & this->seed.uint32t();
    p.position = mathfu::vec3(
            this->seed.Uniform() * this->aniProp.emissionAreaY * 0.5,
            this->seed.Uniform() * this->aniProp.emissionAreaY * 0.5,
            0
    );
    float velocity = this->CalcVelocity();
    float zSource = this->aniProp.zSource;

    if (zSource < 0.001) {
        float polar = this->aniProp.verticalRange * this->seed.Uniform();
        float azimuth = this->aniProp.horizontalRange * this->seed.Uniform();
        float sinPolar = sin(polar);
        float sinAzimuth = sin(azimuth);
        float cosPolar = cos(polar);
        float cosAzimuth = cos(azimuth);
        p.velocity = mathfu::vec3(
            cosAzimuth * sinPolar * velocity,
            sinAzimuth * sinPolar * velocity,
            cosPolar * velocity
        );
    }
    else {
        mathfu::vec3 r0 = mathfu::vec3(0, 0, zSource);
        r0 = p.position - r0;
        if (r0.Length() > 0.0001) {
            r0 = r0.Normalized();
            p.velocity = r0 * velocity;
        }
    }
}
