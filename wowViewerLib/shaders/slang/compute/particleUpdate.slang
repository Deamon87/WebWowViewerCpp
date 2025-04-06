import glsl;

#include "../common/particle/particleUpdate.slang"
#include "../common/particle/particleEmitterState.slang"
#include "../common/animationInputs.slang"

[vk::push_constant] 
uint particleEmitterIdx;

[shader("compute")]
[numthreads(12,1,1)]
void main() {
    //ParticleEmitter index 
    //uint peIdx = gl_GlobalInvocationID.x;
    uint peIdx = particleEmitterIdx;
    uint particleIdx = gl_GlobalInvocationID.x;

    StepUpdate_update(
        particleCpuStates[peIdx],
        particleM2Data[particleCpuStates[peIdx].particleM2DataIndex],
        particleGpuStates[peIdx],
        particleCpuStates[peIdx].deltaTime,
        currParticlesBuffer[particleIdx]
    );
}