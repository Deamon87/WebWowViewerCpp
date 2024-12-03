#include "particleGenerator.glsl"

void CreateParticle(
    // State of emitter
    in GeneratorAniProp aniProp,
    in ParticleM2Data m_data,
    in mat4 m_emitterModelMatrix,
    in vec3 burstVec,
    inout CRndSeed m_seed,


    //Actual parameters
    inout CParticle2 p,
    in float delta
) {
    p.isDead = 0;

    if (m_particleData.emitterType == 1) {
        // Plane
        PlaneGenerator__CreateParticle(aniProp, m_particleData, m_seed, p, delta);
    } else if (m_particleData.emitterType == 2) {
        //Sphere
        CSphereGenerator__CreateParticle(aniProp, m_particleData, m_seed, p, delta);
    } else if (m_particleData.emitterType == 3) {
        //TODO: Spline
    }


    if (!((m_data.flags & 0x10) > 0)) {
        p.position = (m_emitterModelMatrix * vec4(p.position, 1.0f)).xyz();
        p.velocity = (m_emitterModelMatrix * vec4(p.velocity, 0.0f)).xyz();
        if ((m_data.flags & 0x2000) > 0) {
            // Snap to ground; set z to 0:
            p.position.z = 0.0;
        }
    }
    if ((m_data.flags & 0x40) > 0) {
        float speedMul = 1.0f + aniProp.speedVariation * m_seed.Uniform();
        vec3 r0 = burstVec * speedMul;
        p.velocity += r0;
    }
    if ((m_data.flags & 0x10000000) > 0) {
        for (int i = 0; i < 2; i++) {
            p.texPos[i].x = m_seed.UniformPos();
            p.texPos[i].y = m_seed.UniformPos();

            vec2 v2 = m_data.multiTextureParam1[i] * m_seed.Uniform();
            p.texVel[i] = v2 + m_data.multiTextureParam0[i];
        }
    }
}

void StepUpdate_emit(
    // State of emitter
    in GeneratorAniProp aniProp,
    in ParticleM2Data m_particleData,
    in mat4 m_emitterModelMatrix,
    in vec3 burstVec,
    inout CRndSeed m_seed,
    inout emission,

    // Actual paramaters
    uint pAmount,                   // current amount of particles
    uint maxAmount,                 // max amount of particles
    float delta,                    // deltaTime
    inout CParticle2[] particlesCurr  // Buffer for this frame
) {
    float rate = GetEmissionRate(aniProp, m_particleData);
    emission += delta * rate;

    while (emission > 1.0f) {
        if (pAmount < maxAmount) {
            CreateParticle(
                aniProp,
                m_particleData,
                m_emitterModelMatrix,
                burstVec,
                m_seed,
                m_particlesCurr[pAmount++],
                delta
            );
        } else {
            break;
        }
        emission -= 1.0f;
    }
}
