import glsl;

#include "../common/particle/particleEmitterState.slang"
#include "../common/particle/particleEmit.slang"

// Process particles in blocks of 128
[shader("compute")]
[numthreads(1,1,1)]
void main(void)
{
    //ParticleEmitter index
    uint peIdx = gl_GlobalInvocationID.x;
    
    float deltaTime = particleCpuStates[peIdx].deltaTime;

    uint simCount = particleGpuStates[peIdx].numberOf10msStepUpdates;
    
    for (uint i = 0; i < simCount; i++) {
        StepUpdate_emit(
            particleCpuStates[peIdx],
            particleM2Data[particleCpuStates[peIdx].particleM2DataIndex],
            particleGpuStates[peIdx],
            0.1,
            currParticlesBuffer
        );
    }
}