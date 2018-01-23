//
// Created by deamon on 15.12.17.
//

#include "CParticleGenerator.h"

float CParticleGenerator::GetEmissionRate() {
    float rate = aniProp.emissionRate;
    return rate + seed->Uniform(); //* FixedProp[0];
}

float CParticleGenerator::GetMaxEmissionRate() {
    return aniProp.emissionRate ;//+ FixedProp[0];
}

float CParticleGenerator::GetMaxLifeSpan() {
    return aniProp.lifespan; //+ FixedProp[1];
}

float CParticleGenerator::GetLifeSpan(float m) {
    return aniProp.lifespan ;//+ m * FixedProp[1];
}

void CParticleGenerator::CalcVelocity(CParticle2 *p, animTime_t delta) {
//    float velocity = p->aniProp.emissionSpeed;
//    velocity *= 1.0 + p->aniProp.speedVariation * m_seed.Uniform();
//    return velocity;
}


void CParticleGenerator::Update(animTime_t time, mathfu::mat4 &viewMat) {

}

void CParticleGenerator::CreateParticle(CParticle2 &p, animTime_t delta) {

}

mathfu::vec3 CParticleGenerator::GetGravity() {
    return mathfu::vec3(aniProp.gravity);
}

long CParticleGenerator::GetSpeedVariation() {
    return 0;
}
