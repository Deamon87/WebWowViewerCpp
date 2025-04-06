#ifndef PARICLE_PARTICLE_EMITTER_STATE_SLANG
#define PARICLE_PARTICLE_EMITTER_STATE_SLANG

import glsl; 

#include "./particleCommon.slang"
#include "./rndSeed.slang"

StructuredBuffer<ParticleM2Data, ScalarDataLayout> particleM2Data;
RWStructuredBuffer<ParticleEmitterGPUState, ScalarDataLayout> particleGpuStates;
RWStructuredBuffer<ParticleEmitterCPUState, ScalarDataLayout> particleCpuStates;

RWStructuredBuffer<CParticle2, ScalarDataLayout> currParticlesBuffer; 
StructuredBuffer<CParticle2, ScalarDataLayout> prevParticlesBuffer; 

#endif // PARICLE_PARTICLE_EMITTER_STATE_SLANG