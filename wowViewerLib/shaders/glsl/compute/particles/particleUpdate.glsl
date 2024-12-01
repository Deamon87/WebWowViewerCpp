#include "particleGenerator.glsl"

struct ParticleForces {
    vec3 drift; // 0
    vec3 velocity; // 3
    vec3 position; // 6
    float drag; // 9
};

struct CParticle2 {
    vec3 position;
    float age;
    vec3 velocity;
    int state;
    int seed;
    vec2 texPos[2];
    vec2 texVel[2];
};

void StepUpdate_age(
    // State of emitter
    in GeneratorAniProp aniProp,
    in ParticleM2Data m_particleData,

    // Actual paramaters
    in CParticle2[] particlesLast,  // Buffer for last frame
    uint pAmount,                   // current amount of particles
    uint maxAmount,                 // max amount of particles
    float delta,                    // deltaTime
    out CParticle2[] particlesCurr  // Buffer for this frame
) {
    ParticleForces forces;

    int pAliveIdx = 0;

    for (int i = 0; i < pAmount; i++) {
        CParticle2 p = particlesLast[i];
        p.age = p.age + delta;

        bool isDead = ( p.age > (max(GetLifeSpan(aniProp, m_particleData, p.seed), 0.001f)));
        if (!isDead) {
            particlesCurr[pAliveIdx++] = p;
        }
    }
}