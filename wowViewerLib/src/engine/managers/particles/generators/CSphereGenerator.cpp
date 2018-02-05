//
// Created by deamon on 15.12.17.
//

#include "CSphereGenerator.h"

void CSphereGenerator::CreateParticle(CParticle2 &p, animTime_t delta) {

    float dvary = delta * this->seed.UniformPos();
    float life = this->seed.Uniform();
    p.lifespan = life;
    float lifespan = this->GetLifeSpan(life);
    if (lifespan < 0.001) {
        lifespan = 0.001;
    }
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

    mathfu::vec3 r0;
    if (zSource < 0.001) {
        if (this->particlesGoUp) {
            r0 = mathfu::vec3(0,0,1);
        }
    }
    else {
        r0 = mathfu::vec3(0, 0, zSource);
        r0 = p.position - r0;
        if (r0.Length() > 0.0001) {
            r0 = r0.Normalized();
        }
    }
    p.velocity = r0 * velocity;
}
