//
// Created by deamon on 15.12.17.
//

#include "CParticleGenerator.h"

float CParticleGenerator::GetEmissionRate() {
    float rate = aniProp.emissionRate;
    return rate + seed.Uniform() * m_particleData->old.emissionRateVary;
}

float CParticleGenerator::GetMaxEmissionRate() {
    return aniProp.emissionRate + m_particleData->old.emissionRateVary;
}

float CParticleGenerator::GetMaxLifeSpan() {
    return aniProp.lifespan + m_particleData->old.lifespanVary;
}

float CParticleGenerator::GetLifeSpan(int16_t state) {
    return aniProp.lifespan + (state * 0.000030518509f) * m_particleData->old.lifespanVary;
}

float CParticleGenerator::CalcVelocity() {
    float velocity = aniProp.emissionSpeed;
    velocity *= 1.0 + aniProp.speedVariation * seed.Uniform();
    return velocity;
}


void CParticleGenerator::Update(animTime_t time, mathfu::mat4 &viewMat) {

}
mathfu::vec3 CParticleGenerator::GetGravity() {
    return mathfu::vec3(aniProp.gravity);
}

float CParticleGenerator::GetSpeedVariation() {
    return 0;
}
