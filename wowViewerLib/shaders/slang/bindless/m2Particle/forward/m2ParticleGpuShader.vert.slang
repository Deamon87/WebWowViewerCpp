//
// Pull-model GPU particle vertex shader: replaces the CPU quad-expansion
// (ParticleEmitter::prepearAndUpdateBuffers / CalculateParticlePreRenderData /
// buildVertex1 / buildVertex2 / BuildQuadT3 in particleEmitter.cpp:632-1172).
//
// No vertex attributes: gl_VertexIndex encodes particle slot and quad corner via
// the shared particle index buffer (4 verts/quad, 6 indices: 0,1,2 / 3,2,1).
// Emitters with head+tail (flags 0x60000) draw 2 quads per particle.
//
// Positions are computed directly in view space (CPU round-trips through world
// space via inverseViewMatrix; scene.uLookAtMat is the same matrix, so the result
// is identical without the round trip).
//
#version 450

import glsl;
#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec4 vColor;
layout(location = 2) out vec2 vTexcoord0;
layout(location = 3) out vec2 vTexcoord1;
layout(location = 4) out vec2 vTexcoord2;
layout(location = 5) out float vAlphaCutoff;
layout(location = 6) out flat float vObjectIdBits;

#include "../../../common/commonLightFunctions.slang"
#include "../../../common/commonFogFunctions.slang"
#include "../../../common/commonUboSceneData.slang"

#include "../../../common/particle/particleGpu.slang"
#include "../../../common/particle/crndSeed.slang"

// Per-emitter data (extended meshParticleWideBlockPS — see UniformBufferStructures.h)
layout (std140, set = 1, binding = 0) uniform meshWideBlockPS {
    vec4 uAlphaTest_alphaMult_colorMult;
    ivec4 uPixelShaderBlendModev;
    ivec4 gpuBind0; // stateIndex, particleOffset (unused; VS reads the state's pool-absolute offsets), capacity, staticsIndex
    ivec4 gpuBind1; // propsIndex, colorReplOffset, objectId, quadsPerParticle
    ivec4 gpuBind2; // valuesVec4Offset, valuesFloatOffset, partTimesOffset, unused
};

layout(std430, set=3, binding=0) StructuredBuffer<GpuParticleState, ScalarDataLayout> particleStates;
layout(std430, set=3, binding=1) StructuredBuffer<GpuM2ParticleStatic, ScalarDataLayout> particleStatics;
layout(std430, set=3, binding=2) StructuredBuffer<GpuM2ParticleFrameProps, ScalarDataLayout> particleProps;
layout(std430, set=3, binding=3) StructuredBuffer<GpuParticle, ScalarDataLayout> particlesA;
layout(std430, set=3, binding=4) StructuredBuffer<GpuParticle, ScalarDataLayout> particlesB;
layout(std430, set=3, binding=5) StructuredBuffer<uint, ScalarDataLayout> partTimes;
layout(std430, set=3, binding=6) StructuredBuffer<vec4, ScalarDataLayout> trackValuesVec4;
layout(std430, set=3, binding=7) StructuredBuffer<float, ScalarDataLayout> trackValuesFloat;
layout(std430, set=3, binding=8) StructuredBuffer<float, ScalarDataLayout> randTable;
layout(std430, set=3, binding=9) StructuredBuffer<vec4, ScalarDataLayout> particleColorRepl;

bool feq(float a, float b) {
    return abs(a - b) <= 0.001;
}

// High 32 bits of a 32x32 unsigned multiply, computed with 16-bit limbs.
// Avoids the SPIR-V Int64 capability: shaderInt64 is not enabled on the device
// (VUID-VkShaderModuleCreateInfo-pCode-08740).
uint umulhi32(uint a, uint b) {
    uint aLo = a & 0xFFFFu, aHi = a >> 16;
    uint bLo = b & 0xFFFFu, bHi = b >> 16;
    uint mid1 = aHi * bLo;
    uint mid2 = aLo * bHi;
    uint carry = (((aLo * bLo) >> 16) + (mid1 & 0xFFFFu) + (mid2 & 0xFFFFu)) >> 16;
    return aHi * bHi + (mid1 >> 16) + (mid2 >> 16) + carry;
}

GpuParticle loadParticle(int stateIndex, int slot) {
    GpuParticleState state = particleStates[stateIndex];
    // Pool-absolute region bases; A/B chunk offsets may differ
    uint idx = uint(((state.curBuf == 0) ? state.particleOffset : state.particleOffsetB) + slot);
    return (state.curBuf == 0) ? particlesA[idx] : particlesB[idx];
}

void emitDegenerate() {
    // z > w -> clipped by the rasterizer; alpha 0 as a belt-and-braces
    gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
    vPosition = vec3(0.0);
    vColor = vec4(0.0);
    vTexcoord0 = vec2(0.0);
    vTexcoord1 = vec2(0.0);
    vTexcoord2 = vec2(0.0);
    vAlphaCutoff = 0.0;
    vObjectIdBits = 0.0;
}

struct AgeDependentValues {
    vec3 timedColor;
    float alpha;
    vec2 particleScale;
    int timedHeadCell;
    int timedTailCell;
    float alphaCutoff;
};

// ParticleEmitter::fillTimedParticleData (particleEmitter.cpp:1013-1120)
AgeDependentValues fillTimedParticleDataGpu(
    in GpuM2ParticleStatic st,
    in GpuM2ParticleFrameProps props,
    in GpuParticleState state,
    in GpuParticle p,
    int valuesVec4OffsetBase,
    in int valuesFloatOffsetBase,
    int partTimesOffsetBase,
    int colorReplOffset) {

    AgeDependentValues adv;

    float maxLifeSpan = props.lifespan + st.lifespanVary; // GetMaxLifeSpan
    float percentTime = p.age / maxLifeSpan;
    if (min(percentTime, 1.0) <= 0.0) {
        percentTime = 0.0;
    } else if (percentTime >= 1.0) {
        percentTime = 1.0;
    }

    CRndSeed rand = CRndSeed_create(p.seed);

    bool colorReplProvided = false;
    vec3 timedColor = vec3(255.0);
    if (st.particleColorIndex >= 11 && st.particleColorIndex <= 13 && colorReplOffset >= 0
        && st.colorTrackSlice.elemCount >= 3) {
        colorReplProvided = true;
        int replBase = colorReplOffset + (st.particleColorIndex - 11) * 3;

        float t0 = float(partTimes[uint(partTimesOffsetBase) + st.colorTrackSlice.timeOffset + 0]) / 32767.0;
        float t1 = float(partTimes[uint(partTimesOffsetBase) + st.colorTrackSlice.timeOffset + 1]) / 32767.0;
        float t2 = float(partTimes[uint(partTimesOffsetBase) + st.colorTrackSlice.timeOffset + 2]) / 32767.0;

        vec4 timedValue;
        if (percentTime < t1) {
            float alpha = clamp((t1 - percentTime) / (t1 - t0), 0.0, 1.0);
            timedValue = (particleColorRepl[replBase + 1] - particleColorRepl[replBase + 0]) * (1.0 - alpha)
                       + particleColorRepl[replBase + 0];
        } else {
            float alpha = clamp((t2 - percentTime) / (t2 - t1), 0.0, 1.0);
            timedValue = (particleColorRepl[replBase + 2] - particleColorRepl[replBase + 1]) * (1.0 - alpha)
                       + particleColorRepl[replBase + 1];
        }
        timedColor = timedValue.xyz;
    }
    if (!colorReplProvided) {
        timedColor = evalM2PartTrackVec4(st.colorTrackSlice, percentTime, partTimes, partTimesOffsetBase,
                                         trackValuesVec4, valuesVec4OffsetBase,
                                         vec4(255.0, 255.0, 255.0, 0.0)).xyz / 255.0;
    }

    adv.timedColor = timedColor;
    adv.particleScale = evalM2PartTrackVec4(st.scaleTrackSlice, percentTime, partTimes, partTimesOffsetBase,
                                            trackValuesVec4, valuesVec4OffsetBase,
                                            vec4(1.0, 1.0, 0.0, 0.0)).xy;
    adv.alpha = evalM2PartTrackFloat(st.alphaTrackSlice, percentTime, partTimes, partTimesOffsetBase,
                                     trackValuesFloat, valuesFloatOffsetBase, 1.0);

    if (st.headCellTrackSlice.elemCount > 0) {
        adv.timedHeadCell = evalM2PartTrackCell(st.headCellTrackSlice, percentTime, partTimes, partTimesOffsetBase,
                                                trackValuesFloat, valuesFloatOffsetBase, 0);
        adv.timedHeadCell = st.textureIndexMask & (adv.timedHeadCell + int(state.randomizedTextureIndexMask));
    } else {
        if ((st.flags & 0x10000) != 0) {
            // (textureIndexMask + 1) * uint32 / 2^32
            adv.timedHeadCell = int(umulhi32(uint(st.textureIndexMask) + 1u, CRndSeed_uint32t(rand)));
        } else {
            adv.timedHeadCell = 0;
        }
    }

    adv.timedTailCell = evalM2PartTrackCell(st.tailCellTrackSlice, percentTime, partTimes, partTimesOffsetBase,
                                            trackValuesFloat, valuesFloatOffsetBase, 0);
    adv.alphaCutoff = 0.0;
    if (st.hasExp2 != 0) {
        adv.alphaCutoff = evalM2PartTrackFloat(st.alphaCutoffSlice, percentTime, partTimes, partTimesOffsetBase,
                                               trackValuesFloat, valuesFloatOffsetBase, 0.0);
    }

    adv.timedTailCell = (adv.timedTailCell + int(state.randomizedTextureIndexMask)) & st.textureIndexMask;

    float scaleMultiplier = 1.0;
    if ((st.flags & 0x80000) != 0) {
        scaleMultiplier = max(1.0 + CRndSeed_Uniform(rand) * st.scaleVary[1], 0.000099999997);
        adv.particleScale.x = max(1.0 + CRndSeed_Uniform(rand) * st.scaleVary[0], 0.000099999997)
                              * adv.particleScale.x;
    } else {
        scaleMultiplier = max(1.0 + CRndSeed_Uniform(rand) * st.scaleVary[0], 0.000099999997);
        adv.particleScale.x = scaleMultiplier * adv.particleScale.x;
    }
    adv.particleScale.y = scaleMultiplier * adv.particleScale.y;

    return adv;
}

// ParticleEmitter::GetSpin (particleEmitter.cpp:950-970)
void getSpin(in GpuM2ParticleStatic st, in GpuParticle p, out float baseSpin, out float deltaSpin) {
    if (!feq(st.baseSpin, 0.0) || !feq(st.spinVary, 0.0)) {
        CRndSeed rand = CRndSeed_create(p.seed);

        if (feq(st.baseSpinVary, 0.0)) {
            baseSpin = st.baseSpin;
        } else {
            baseSpin = st.baseSpin + CRndSeed_Uniform(rand) * st.baseSpinVary;
        }

        if (feq(st.spinVary, 0.0)) {
            deltaSpin = st.spin;
        } else {
            deltaSpin = st.spin + CRndSeed_Uniform(rand) * st.spinVary;
        }
    } else {
        baseSpin = st.baseSpin;
        deltaSpin = st.spin;
    }
}

[shader("vertex")]
void main() {
    int stateIndex      = gpuBind0.x;
    int capacity        = gpuBind0.z;
    int staticsIndex    = gpuBind0.w;
    int propsIndex      = gpuBind1.x;
    int colorReplOffset = gpuBind1.y;
    int objectId        = gpuBind1.z;
    int quadsPerParticle = gpuBind1.w;

    GpuParticleState state = particleStates[stateIndex];
    GpuM2ParticleStatic st = particleStatics[staticsIndex];
    GpuM2ParticleFrameProps props = particleProps[propsIndex];

    int vertIdx = int(gl_VertexIndex);
    int quadIdx = vertIdx >> 2;                 // quadsPerParticle quads per particle
    int corner  = vertIdx & 3;
    int slot    = quadIdx / quadsPerParticle;   // particle index
    bool isTail = quadsPerParticle > 1 && (quadIdx % quadsPerParticle) == 1;

    // Dead / out-of-range / unborn slots render as clipped degenerate triangles
    if (slot >= state.particleCount) {
        emitDegenerate();
        return;
    }

    GpuParticle p = loadParticle(stateIndex, slot);
    if (p.isDead != 0) {
        emitDegenerate();
        return;
    }

    // ---- CalculateParticlePreRenderData ----
    float twinkle = st.twinklePercent;
    float twinkleMin = st.twinkleScaleMin;
    float twinkleVary = st.twinkleScaleMax - twinkleMin;

    int rndIdx = 0;
    if (twinkle < 1.0 || !feq(twinkleVary, 0.0)) {
        rndIdx = 0x7f & (int(p.age * st.twinkleSpeed) + int(p.seed));
    }

    if (twinkle < randTable[rndIdx]) {
        emitDegenerate();
        return;
    }

    float maxLifeSpan = props.lifespan + st.lifespanVary;
    if (p.age > maxLifeSpan) {
        emitDegenerate();
        return;
    }

    AgeDependentValues adv = fillTimedParticleDataGpu(st, props, state, p,
                                                      gpuBind2.x /*valuesVec4Offset*/,
                                                      gpuBind2.y /*valuesFloatOffset*/,
                                                      gpuBind2.z /*partTimesOffset*/,
                                                      colorReplOffset);

    float weight = twinkleVary * randTable[rndIdx] + twinkleMin;
    adv.particleScale *= weight;

    if ((st.flags & 0x20) != 0) {
        adv.particleScale *= state.inheritedScale;
    }

    mat4 s_particleToView = matFromArray16(state.s_particleToView);
    vec3 pPos = arrToVec3(p.position);
    vec3 pVel = arrToVec3(p.velocity);
    vec3 particleCenter = (s_particleToView * vec4(pPos, 1.0)).xyz;

    // ---- buildVertex1 / buildVertex2 ----
    int timedCell = isTail ? adv.timedTailCell : adv.timedHeadCell;

    int coefXInt = (timedCell & st.textureColMask);
    float coefXf = float(coefXInt); // coefXInt >= 0 always (mask is non-negative)
    vec2 texScaleVec = vec2(
        coefXf * st.texScaleX,
        float(timedCell >> st.textureColBits) * st.texScaleY
    );

    float baseSpin = 0.0;
    float deltaSpin = 0.0;
    getSpin(st, p, baseSpin, deltaSpin);

    vec3 m0 = vec3(0.0);
    vec3 m1 = vec3(0.0);
    bool dataFilled = false;
    bool force0x1000 = false;
    int v20 = 0;

    if (isTail) {
        // ---- buildVertex2 (tail quad, velocity-stretched) ----
        float trailTime = st.tailLength;
        if ((st.flags & 0x400) != 0) {
            trailTime = min(p.age, trailTime);
        }
        vec3 viewVel = (s_particleToView * vec4(-pVel, 0.0)).xyz * trailTime;
        vec3 screenVel = vec3(viewVel.xy, 0.0);

        if (dot(screenVel, screenVel) > 0.0001) {
            float invScreenVelMag = 1.0 / length(screenVel);
            adv.particleScale = adv.particleScale * invScreenVelMag;
            screenVel = vec3(screenVel.xy * adv.particleScale, 0.0);
            m1 = vec3(-screenVel.y, screenVel.x, 0.0);
            m0 = viewVel * 0.5;
            particleCenter += m0;
        } else {
            m0 = vec3(adv.particleScale.x * 0.05, 0.0, 0.0);
            m1 = vec3(0.0, adv.particleScale.y * 0.05, 0.0);
        }
        dataFilled = true;
    } else {
        // ---- buildVertex1 (head quad) ----
        float velLenSq = dot(pVel, pVel);
        if ((st.flags & 0x4) != 0) {
            if (velLenSq > 0.00000023841858) {
                v20 = 1;
                if ((st.flags & 0x1000) == 0) {
                    vec3 viewSpaceVel = (s_particleToView * vec4(-pVel, 0.0)).xyz;
                    float translatedVelLeng = dot(viewSpaceVel, viewSpaceVel);
                    float v30 = (translatedVelLeng <= 0.00000023841858) ? 0.0 : 1.0 / sqrt(translatedVelLeng);

                    vec3 viewSpaceVelNorm = viewSpaceVel * v30;

                    m0 = viewSpaceVelNorm * adv.particleScale.x;
                    m1 = vec3(
                        viewSpaceVelNorm.y * adv.particleScale.y,
                        -(viewSpaceVelNorm.x * adv.particleScale.y),
                        0.0
                    );
                    dataFilled = true;
                } else {
                    force0x1000 = true;
                }
            }
        }

        if (((st.flags & 0x1000) != 0 || force0x1000) && !dataFilled) {
            // model-plane billboard with optional velocity alignment
            mat3 transformedQuadToViewMat = mat3(
                state.p_quadToView0.xyz,
                state.p_quadToView1.xyz,
                state.p_quadToView2.xyz
            );
            float v39 = adv.particleScale.x;
            if (v20 != 0) {
                vec3 minusVel = -pVel;
                float translatedVelLeng = dot(minusVel, minusVel);
                float v47 = (translatedVelLeng <= 0.00000023841858) ? 0.0 : 1.0 / sqrt(translatedVelLeng);

                mat3 velAlign = mat3(
                    minusVel.x * v47, minusVel.y * v47, 0.0,
                    -minusVel.y * v47, minusVel.x * v47, 0.0,
                    0.0, 0.0, 1.0
                );
                transformedQuadToViewMat = transformedQuadToViewMat * velAlign;

                if (v47 > 0.00000023841858) {
                    v39 = adv.particleScale.x * (1.0 / sqrt(velLenSq) / v47);
                }
            }

            m0 = transformedQuadToViewMat[0] * v39;
            m1 = transformedQuadToViewMat[1] * adv.particleScale.y;
            deltaSpin = m1.x;
            dataFilled = true;
            if (!feq(st.spin, 0.0) || !feq(st.spinVary, 0.0)) {
                float theta = baseSpin + deltaSpin * p.age;
                if ((st.flags & 0x200) != 0 && (p.seed & 1u) != 0u) {
                    theta = -theta;
                }

                vec3 zAxis = state.p_quadToViewZVector.xyz;

                // mathfu::quat::FromAngleAxis(theta, zAxis).ToMatrix() — via the shared
                // mathfu-exact quat->matrix helper (quat = vec4(xyz, w))
                vec4 spinQuat = vec4(normalize(zAxis) * sin(theta * 0.5), cos(theta * 0.5));
                mat3 rotMat = mat3(
                    quatToMat4(spinQuat)[0].xyz,
                    quatToMat4(spinQuat)[1].xyz,
                    quatToMat4(spinQuat)[2].xyz
                );

                m0 = rotMat * m0;
                m1 = rotMat * vec3(deltaSpin, m1.y, m1.z);
            }
        }

        if (!dataFilled) {
            if (!feq(st.spin, 0.0) || !feq(st.spinVary, 0.0)) {
                float theta = baseSpin + deltaSpin * p.age;
                if ((st.flags & 0x200) != 0 && (p.seed & 1u) != 0u) {
                    theta = -theta;
                }

                float cosTheta = cos(theta);
                float sinTheta = sin(theta);

                m0 = vec3(cosTheta * adv.particleScale.x,
                          sinTheta * adv.particleScale.x, 0.0);
                m1 = vec3(-sinTheta * adv.particleScale.y,
                          cosTheta * adv.particleScale.y, 0.0);

                if ((st.flags & 0x8000000) != 0) {
                    particleCenter += vec3(m1.x, m1.y, 0.0);
                }
            } else {
                m0 = vec3(adv.particleScale.x, 0.0, 0.0);
                m1 = vec3(0.0, adv.particleScale.y, 0.0);
            }
        }
    }

    // ---- BuildQuadT3 ----
    const float vxs[4] = { -1.0, -1.0, 1.0, 1.0 };
    const float vys[4] = { 1.0, -1.0, 1.0, -1.0 };
    const float txs[4] = { 0.0, 0.0, 1.0, 1.0 };
    const float tys[4] = { 0.0, 1.0, 0.0, 1.0 };

    vec3 viewPos = m0 * vxs[corner] + m1 * vys[corner] + particleCenter;

    gl_Position = scene.uPMatrix * vec4(viewPos, 1.0);

    vPosition = viewPos;
    vColor = vec4(adv.timedColor, adv.alpha);
    vTexcoord0 = vec2(txs[corner] * st.texScaleX + texScaleVec.x,
                      tys[corner] * st.texScaleY + texScaleVec.y);

    float2 texPos0 = arrToVec2(p.texPos0);
    float2 texPos1 = arrToVec2(p.texPos1);
    vTexcoord1 = vec2(txs[corner] * st.multiTextureParamX0 + texPos0.x,
                      tys[corner] * st.multiTextureParamX0 + texPos0.y);
    vTexcoord2 = vec2(txs[corner] * st.multiTextureParamX1 + texPos1.x,
                      tys[corner] * st.multiTextureParamX1 + texPos1.y);

    vAlphaCutoff = adv.alphaCutoff;
    vObjectIdBits = intBitsToFloat(objectId);
}
