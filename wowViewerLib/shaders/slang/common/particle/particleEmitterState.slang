import glsl; 

#include "./particleCommon.slang"
#include "./rndSeed.slang"

struct ParticleEmitterGPUState {
    float emission; 
    CRndSeed seed;
    vec3 m_prevPosition;
    float burstTime;
    vec3 burstVec;
    vec3 m_fullPosDeltaSinceLastUpdate;
    float m_inheritedScale;

    vec3 m_deltaPosition;
    mat4 m_emitterModelMatrix;
    
    uint numberOf10msStepUpdates;
    float endDeltaTime;

    int particlesCount;
};

struct ParticleEmitterCPUState {
    GeneratorAniProp aniProp;
    uint particleM2DataIndex;
}

StructuredBuffer<ParticleM2Data, ScalarDataLayout> particleM2Data;
RWStructuredBuffer<ParticleEmitterGPUState, ScalarDataLayout> particleGpuStates;
RWStructuredBuffer<ParticleEmitterCPUState, ScalarDataLayout> particleCpuStates;

RWStructuredBuffer<CParticle2, ScalarDataLayout> particlesBuffer; 