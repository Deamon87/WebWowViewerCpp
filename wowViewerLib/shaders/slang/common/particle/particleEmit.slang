#include "./particleGenerator.slang"

void CreateParticle(
    // State of emitter
    in ParticleEmitterCPUState cpuState,
    in ParticleM2Data m_data,
    inout ParticleEmitterGPUState gpuState,

    //Actual parameters
    inout CParticle2 p,
    in float delta
) {
    p.isDead = 0;

    if (m_data.emitterType == 1) {
        // Plane
        PlaneGenerator__CreateParticle(cpuState.aniProp, m_data, gpuState.m_seed, p, delta);
    } else if (m_data.emitterType == 2) {
        //Sphere
        CSphereGenerator__CreateParticle(cpuState.aniProp, m_data, gpuState.m_seed, p, delta);
    } else if (m_data.emitterType == 3) {
        //TODO: Spline
    }


    if (!((m_data.flags & 0x10) > 0)) {
        p.position = (gpuState.m_emitterModelMatrix * vec4(p.position, 1.0f)).xyz;
        p.velocity = (gpuState.m_emitterModelMatrix * vec4(p.velocity, 0.0f)).xyz;
        if ((m_data.flags & 0x2000) > 0) {
            // Snap to ground; set z to 0:
            p.position.z = 0.0;
        }
    }
    if ((m_data.flags & 0x40) > 0) {
        float speedMul = 1.0f + cpuState.aniProp.speedVariation * gpuState.m_seed.Uniform();
        vec3 r0 = gpuState.burstVec * speedMul;
        p.velocity += r0;
    }
    if ((m_data.flags & 0x10000000) > 0) {
        for (int i = 0; i < 2; i++) {
            p.texPos[i].x = gpuState.m_seed.UniformPos();
            p.texPos[i].y = gpuState.m_seed.UniformPos();

            vec2 v2 = m_data.multiTextureParam1[i] * gpuState.m_seed.Uniform();
            p.texVel[i] = v2 + m_data.multiTextureParam0[i];
        }
    }
}

void StepUpdate_emit(
    // State of emitter
    in ParticleEmitterCPUState cpuState,
    in ParticleM2Data m_particleData,
    inout ParticleEmitterGPUState gpuState,
    
    // Actual paramaters
    float delta,                    // deltaTime
    RWStructuredBuffer<CParticle2, ScalarDataLayout> particlesCurr  // Buffer for this frame
) {
    float rate = GetEmissionRate(cpuState.aniProp, m_particleData, gpuState.m_seed);
    gpuState.emission += delta * rate;

    while (gpuState.emission > 1.0f) {
        if (gpuState.particlesCount < cpuState.maxAmount) {
            CreateParticle(
                cpuState,
                m_particleData,
                gpuState,
                particlesCurr[gpuState.particlesCount++],
                delta
            );
        } else {
            break;
        }
        gpuState.emission -= 1.0f;
    }
}
