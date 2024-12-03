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
    int isDead;
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

        p.isDead = (p.isDead > 0 || p.age > (max(GetLifeSpan(aniProp, m_particleData, p.seed), 0.001f))) ? 1 : 0;
        if (p.isDead == 0) {
            particlesCurr[pAliveIdx++] = p;
        }
    }
}

void CalculateForces(
    // State of emitter
    in GeneratorAniProp aniProp,
    in ParticleM2Data m_particleData,
    // Actual paramaters
    out ParticleForces forces,
    in float delta
) {
    forces.drift = vec3(m_particleData.WindVector) * vec3(delta);

    vec3 g = GetGravity(aniProp, m_particleData);
    forces.velocity = g * vec3(delta);
    forces.position = g * vec3(delta * delta * 0.5f);
    forces.drag = m_particleData.drag * delta;
}

bool UpdateParticle(
    // State of emitter
    in GeneratorAniProp aniProp,
    in ParticleM2Data m_particleData,
    in mat4 m_emitterModelMatrix,
    in vec3 m_deltaPosition,

    // Actual paramaters
    inout CParticle2 p,
    float delta,
    in const ParticleForces forces
) {

    if ((m_particleData.flags & 0x10000000) > 0) {
        for (int i = 0; i < 2; i++) {
            // s = frac(s + v * dt)
            float val = (p.texPos[i].x + delta * p.texVel[i].x);
            p.texPos[i].x = (val - floorf(val));

            val = (p.texPos[i].y + delta * p.texVel[i].y);
            p.texPos[i].y = (val - floorf(val));
        }
    }

    p.velocity = p.velocity + forces.drift;


    //MoveParticle
    if (((m_particleData.flags & 0x4000) > 0) && (2 * delta < p.age)) {
        p.position = p.position + m_deltaPosition;
    }

    vec3 r0 = p.velocity * vec3(delta,delta,delta); // v*dt

    p.velocity = p.velocity + forces.velocity;
    p.velocity = p.velocity *  (1.0f - forces.drag);

    p.position = p.position + r0 + forces.position;
//    p.position = p.position + forces.position;

    if (m_particleData.emitterType == 2 && ((m_particleData.flags & 0x80) > 0)) {
        vec3 r1 = p.position;
        if ((m_particleData.flags & 0x10) > 0) {
            if (dot(r1, r0) > 0) {
                return false;
            }
        } else {
            r1 = p.position - m_emitterModelMatrix[3].xyz;
            if (dot(r1, r0) > 0) {
                return false;
            }
        }
    }

    return true;
}

    //StepUpdate_update
void StepUpdate_update(
    inout CParticle2 p //particle that's being updated
) {
    if (!UpdateParticle(p, delta, forces)) {
        p.isDead = 1;
    }
}