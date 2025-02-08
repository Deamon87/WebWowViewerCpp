import glsl;

import "../common/particle/particleEmitterState.slang";

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
__generic<T : __BuiltinFloatingPointType, let N:int>
[__readNone]
[ForceInline]
T lengthSquared(vector<T, N> x) {
    return dot(x, x);
}

__generic<T : __BuiltinFloatingPointType>
[ForceInline]
inline matrix<T, 3, 3> ToRotationMatrix(const matrix<T, 4> m) {
    return matrix<T, 3, 3>(
        m[0].xyz,
        m[1].xyz,
        m[2].xyz
    );
}

void calculateQuadToViewEtc(
    in ParticleEmitterCPUState cpuState,
    in ParticleM2Data m_data,
    inout ParticleEmitterGPUState gpuState
) {
    if ((m_data.flags & 0x10) > 0) {
        gpuState.s_particleToView = cpuState.viewMatrix * gpuState.m_emitterModelMatrix ;
    } else {
    //    if (a1 != nullptr) {
        //    gpuState.s_particleToView = cpuState.viewMatrix * (*a1);
    //    } else {
           gpuState.s_particleToView = cpuState.viewMatrix;
    //    }
    }

    if ((m_data.flags & 0x1000) > 0) {
        gpuState.p_quadToView = ToRotationMatrix(gpuState.s_particleToView);
 
        if (((m_data.flags & 0x10) > 0) && (abs(gpuState.m_inheritedScale) > 0.000000f) ) {
            gpuState.p_quadToView = gpuState.p_quadToView * (1.0f / gpuState.m_inheritedScale);
        }

        gpuState.p_quadToViewZVector = vec4(gpuState.p_quadToView[2], 0);
        if (lengthSquared(gpuState.p_quadToViewZVector) <= 0.00000023841858f) {
            gpuState.p_quadToViewZVector = vec4(0,0,1.0,0);
        } else {
            gpuState.p_quadToViewZVector = normalize(gpuState.p_quadToViewZVector);
        }
    }
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

        ParticleEmitter__InternalUpdate(cpuState, gpuState, delta);
    }

    //call calculateQuadToViewEtc
    //Moved here from prepearAndUpdateBuffers, because it's still a single thread code
    calculateQuadToViewEtc(cpuState, m_data, gpuState);
}

//M2 data
[[vk::binding(1,1)]]
StructuredBuffer<mat4, Std430DataLayout> uPlacementMats;

[[vk::binding(3,1)]]
StructuredBuffer<mat4, Std430DataLayout> uBoneMatrixes;

struct M2InstanceRecordBindless {
    ivec4 placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd;
    ivec4 textureMatricesInd_modelFragmentDatasInd;
};

[[vk::binding(7,1)]]
StructuredBuffer<M2InstanceRecordBindless, Std430DataLayout> instances;

mat4 BuildTranslation(vec3 delta)
{
    mat4 m;
    m[0][0] = 1;
    m[1][1] = 1;
    m[2][2] = 1;
    m[3] = vec4(delta, 1.0);
    return m;
}


const static mat4 particleCoordinatesFix = mat4(
     0,1,0,0,
    -1,0,0,0,
     0,0,1,0,
    0,0,0,1
);

[shader("compute")]
[numthreads(1,1,1)]
void main(void)
{
    //ParticleEmitter index
    uint peIdx = gl_GlobalInvocationID.x;

    //TODO: create a transform matrix from m2 data, that's already present on GPU 

    uint instanceIndex = particleCpuStates[peIdx].instanceIndex;
    M2InstanceRecordBindless m2Instance = instances[instanceIndex];

    int placementMatrixInd = m2Instance.placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd.x;
    mat4 placementMat = uPlacementMats[placementMatrixInd];

    int boneIndex = instances[instanceIndex].placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd.y;
    int particleBone = particleM2Data[particleCpuStates[peIdx].particleM2DataIndex].bone;
    
    mat4 boneMat = uBoneMatrixes[boneIndex + particleBone];


    mat4 particleEmitterMat =
        placementMat * 
        boneMat *
        BuildTranslation(particleM2Data[particleCpuStates[peIdx].particleM2DataIndex].position) *
        particleCoordinatesFix;
        

    //Single thread update from state
    //Code copy from ParticleEmitter::Update 

    ParticleEmitter__Update(
        particleCpuStates[peIdx],
        particleM2Data[particleCpuStates[peIdx].particleM2DataIndex],
        particleGpuStates[peIdx],
        particleCpuStates[peIdx].deltaTime,
        particleEmitterMat
    );
}
