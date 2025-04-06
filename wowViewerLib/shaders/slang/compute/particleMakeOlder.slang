import glsl;

#include "../common/particle/particleEmitterState.slang"
#include "../common/particle/particleUpdate.slang"

[shader("compute")]
[numthreads(1,1,1)]
void main() {
    //ParticleEmitter index
    uint peIdx = gl_GlobalInvocationID.x;

    StepUpdate_age(
        particleCpuStates[peIdx].aniProp,
        particleM2Data[particleCpuStates[peIdx].particleM2DataIndex],
        prevParticlesBuffer,
        particleGpuStates[peIdx].particlesCount,
        particleCpuStates[peIdx].maxAmount,
        particleCpuStates[peIdx].deltaTime,
        currParticlesBuffer 
    );   
}  