#include "../common/particle/particleEmitterState.slang"

void ParticleEmitter__InternalUpdate(
    in ParticleEmitterCPUState cpuState,
    inout ParticleEmitterGPUState gpuState,
    float delta
) {
    delta = max(delta, 0.0f);
    if (delta < 0.1) {
        gpuState.m_deltaPosition = gpuState.m_fullPosDeltaSinceLastUpdate;
        
        gpuState.numberOf10msStepUpdates = 0;
    } else {

        float temp0 = floor(delta / 0.1f);
        delta = (temp0 * -0.1f) + delta;

        int numberOfStepUpdates = min(floor(cpuState.aniProp.lifespan / 0.1f), temp0);

        int simulationsNum = numberOfStepUpdates+1;
        
        gpuState.m_deltaPosition = gpuState.m_fullPosDeltaSinceLastUpdate * (1.0f / simulationsNum);
        gpuState.numberOf10msStepUpdates = numberOfStepUpdates;
    }


    //Step_Update calls are divided into separate shader calls
}

void ParticleEmitter__Update(
    in ParticleEmitterCPUState cpuState,
    in ParticleM2Data m_data,
    inout ParticleEmitterGPUState gpuState,

    float delta, 
    in mat4 transformMat
) {
    gpuState.m_prevPosition = gpuState.m_emitterModelMatrix[3].xyz;

    //From ParticleEmitter::UpdateXform
    {
        //m_invMatTranslationVector is unused
        gpuState.m_emitterModelMatrix = transformMat;

        gpuState.m_inheritedScale = length(gpuState.m_emitterModelMatrix[0].xyz);
    }

    if (delta > 0) {
        if ((m_data.flags & 0x4000) > 0) {
            gpuState.m_fullPosDeltaSinceLastUpdate = gpuState.m_emitterModelMatrix[3].xyz - gpuState.m_prevPosition;
            float x = m_data.followMult * (length(gpuState.m_fullPosDeltaSinceLastUpdate) / delta) + m_data.followBase;
            if (x >= 0.0)
                x = fmin(x, 1.0f);

            gpuState.m_deltaPosition = gpuState.m_fullPosDeltaSinceLastUpdate * x;
        }

        if ((m_data.flags & 0x40) > 0) {
            gpuState.burstTime += delta;
            float frameTime = 30.0 / 1000.0;
            float burstTime = gpuState.burstTime;
            if (burstTime > frameTime) {
                gpuState.burstTime = 0;

                if (gpuState.particlesCount == 0) {
                    float frameAmount = frameTime / burstTime;
                    vec3 dPos = gpuState.m_emitterModelMatrix[3].xyz - gpuState.m_prevPosition;

                    gpuState.burstVec = dPos * vec3(frameAmount * m_data.BurstMultiplier);
                }
                else {
                    gpuState.burstVec = vec3(0, 0, 0);
                }
            }
        }
    }

}

[shader("compute")]
[numthreads(1,1,1)]
void main(void)
{
    //ParticleEmitter index
    uint peIdx = gl_GlobalInvocationID.x;

    //Single thread update from state
    //Code copy from ParticleEmitter::Update 

    
    ParticleEmitter__Update(
        particleCpuStates[peIdx],
        particleM2Data[particleCpuStates[peIdx].particleM2DataIndex],
        particleGpuStates[peIdx],
        0.1,
        
    )
}
