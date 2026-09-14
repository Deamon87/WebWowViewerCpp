//
// GPU M2 animation compute pass.
// One workgroup per M2 object:
//  - threads evaluate bones in parallel, each replaying its parent chain root->leaf
//    (same composition order as the CPU's recursive AnimationManager::calcBoneMatrix),
//    writing into the renderer's bindless bone-matrix SSBO;
//  - a second small loop evaluates the per-frame particle-emitter property tracks
//    (AnimationManager::calcParticleEmitters) into the particle props pool.
//
// Ported from engine/managers/animationManager.cpp and engine/algorithms/animate.h.
//

import glsl;

#include "../common/m2TrackEval.slang"
#include "../common/particle/particleGpu.slang"
#include "../common/ribbonGpu.slang"

layout(std430, set=0, binding=0) StructuredBuffer<uint, ScalarDataLayout> animDispatchList;
layout(std430, set=0, binding=1) StructuredBuffer<GpuM2AnimState, ScalarDataLayout> animStates;

layout(std430, set=0, binding=2) StructuredBuffer<GpuM2BoneStatic, ScalarDataLayout> boneStatics;
layout(std430, set=0, binding=3) StructuredBuffer<GpuM2TrackHeader, ScalarDataLayout> trackHeaders;
layout(std430, set=0, binding=4) StructuredBuffer<GpuM2SeqSlice, ScalarDataLayout> seqSlices;
layout(std430, set=0, binding=5) StructuredBuffer<uint, ScalarDataLayout> trackTimestamps;
layout(std430, set=0, binding=6) StructuredBuffer<vec4, ScalarDataLayout> trackValuesVec4;
layout(std430, set=0, binding=7) StructuredBuffer<float, ScalarDataLayout> trackValuesFloat;
layout(std430, set=0, binding=8) StructuredBuffer<uint, ScalarDataLayout> seqDurations;
layout(std430, set=0, binding=9) StructuredBuffer<uint, ScalarDataLayout> globalLoops;

// The renderer's bindless bone-matrix pool (same buffer the M2 vertex shaders read)
layout(std430, set=0, binding=10) RWStructuredBuffer<mat4, ScalarDataLayout> boneMatrixOut;

// Particle emitter statics + per-frame props output
layout(std430, set=0, binding=11) StructuredBuffer<GpuM2ParticleStatic, ScalarDataLayout> particleStatics;
layout(std430, set=0, binding=12) RWStructuredBuffer<GpuM2ParticleFrameProps, ScalarDataLayout> particleProps;

// Ribbon statics + per-frame props output
layout(std430, set=0, binding=13) StructuredBuffer<GpuM2RibbonStatic, ScalarDataLayout> ribbonStatics;
layout(std430, set=0, binding=14) RWStructuredBuffer<GpuM2RibbonFrameProps, ScalarDataLayout> ribbonProps;

float4x4 getIdentityMatrix()
{
    return float4x4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

// calcAnimationTransform (animationManager.cpp:170-224)
mat4 calcAnimationTransformGpu(in GpuM2AnimState state, in GpuM2BoneStatic bone) {
    mat4 tranformMat = getIdentityMatrix();
    vec3 pivotPoint = bone.pivot;

    tranformMat = tranformMat * mat4(1,0,0,0, 0,1,0,0, 0,0,1,0,
                                     pivotPoint.x, pivotPoint.y, pivotPoint.z, 1);

    if (bone.transTrack >= 0) {
        vec4 transVec = evalTrackBlendVec4(state, trackHeaders[state.trackHeaderOffset + bone.transTrack],
                                           vec4(0,0,0,0), false, state.animFoundInParent,
                                           seqDurations, globalLoops, seqSlices, trackTimestamps, trackValuesVec4);
        tranformMat = tranformMat * mat4(1,0,0,0, 0,1,0,0, 0,0,1,0,
                                         transVec.x, transVec.y, transVec.z, 1);
    }
    if (bone.rotTrack >= 0) {
        vec4 quat = evalTrackBlendVec4(state, trackHeaders[state.trackHeaderOffset + bone.rotTrack],
                                       vec4(0,0,0,1), true, state.animFoundInParent,
                                       seqDurations, globalLoops, seqSlices, trackTimestamps, trackValuesVec4);
        tranformMat = tranformMat * quatToMat4(quat);
    }
    if (bone.scaleTrack >= 0) {
        vec4 scaleResult = evalTrackBlendVec4(state, trackHeaders[state.trackHeaderOffset + bone.scaleTrack],
                                              vec4(1,1,1,1), false, state.animFoundInParent,
                                              seqDurations, globalLoops, seqSlices, trackTimestamps, trackValuesVec4);
        tranformMat = tranformMat * mat4(scaleResult.x,0,0,0, 0,scaleResult.y,0,0, 0,0,scaleResult.z,0,
                                         0,0,0,1);
    }
    tranformMat = tranformMat * mat4(1,0,0,0, 0,1,0,0, 0,0,1,0,
                                     -pivotPoint.x, -pivotPoint.y, -pivotPoint.z, 1);
    return tranformMat;
}

// calcBoneMatrix body given the parent's final matrix (animationManager.cpp:403-650)
mat4 calcBoneMatrixGpu(in GpuM2AnimState state,
                       in GpuM2BoneStatic bone,
                       in mat4 parentBoneMat,
                       in mat4 modelViewMat,
                       in bool isRoot) {
    mat4 parentMat = parentBoneMat;

    // unk_0x1/0x2/0x4 parent fixups — only for non-root bones
    if (!isRoot && (bone.flagsRaw & 0x7) != 0) {
        mat4 modified = parentMat;

        bool unk0x4 = (bone.flagsRaw & 0x4) != 0;
        bool unk0x2 = (bone.flagsRaw & 0x2) != 0;
        bool unk0x1 = (bone.flagsRaw & 0x1) != 0;

        if (unk0x4 && unk0x2) {
            modified[0] = modelViewMat[0];
            modified[1] = modelViewMat[1];
            modified[2] = modelViewMat[2];
        } else if (unk0x4) {
            float coeff = dot(modelViewMat[0].xyz, modelViewMat[0].xyz);
            coeff = (coeff > 0.0000099999997) ? sqrt(dot(modified[0].xyz, modified[0].xyz) / coeff) : 1.0;
            modified[0] = modelViewMat[0] * coeff;

            coeff = dot(modelViewMat[1].xyz, modelViewMat[1].xyz);
            coeff = (coeff > 0.0000099999997) ? sqrt(dot(modified[1].xyz, modified[1].xyz) / coeff) : 1.0;
            modified[1] = modelViewMat[1] * coeff;

            coeff = dot(modelViewMat[2].xyz, modelViewMat[2].xyz);
            coeff = (coeff > 0.0000099999997) ? sqrt(dot(modified[2].xyz, modified[2].xyz) / coeff) : 1.0;
            modified[2] = modelViewMat[2] * coeff;
        } else if (unk0x2) {
            float coeff = dot(modified[0].xyz, modified[0].xyz);
            if (coeff > 0.0000099999997) {
                modified[0] = modelViewMat[0] * (1.0 / sqrt(coeff));
            }
            coeff = dot(modified[1].xyz, modified[1].xyz);
            if (coeff > 0.0000099999997) {
                modified[1] = modelViewMat[1] * (1.0 / sqrt(coeff));
            }
            coeff = dot(modified[2].xyz, modified[2].xyz);
            if (coeff > 0.0000099999997) {
                modified[2] = modelViewMat[2] * (1.0 / sqrt(coeff));
            }

            modified[0] = modified[0] * length(modelViewMat[0].xyz);
            modified[1] = modified[1] * length(modelViewMat[1].xyz);
            modified[2] = modified[2] * length(modelViewMat[2].xyz);
        }
        if (unk0x1) {
            modified[3] = modelViewMat[3];
        } else {
            vec4 pivotVec4 = vec4(bone.pivot, 1.0);
            vec4 pivotVec3 = vec4(bone.pivot, 0.0);

            modified[3] = (parentMat * pivotVec4) - (modified * pivotVec3);
            modified[3].w = 1.0;
        }

        parentMat = modified;
    }

    mat4 currentBoneMat;
    mat4 animatedMatrix = getIdentityMatrix();
    const bool isAnimated = (bone.flagsRaw & 0x280) != 0;
    if (isAnimated) {
        animatedMatrix = calcAnimationTransformGpu(state, bone);
        currentBoneMat = parentMat * animatedMatrix;
    } else {
        currentBoneMat = parentMat;
    }

    uint boneBillboardFlags = bone.flagsRaw & 0x4000078;
    if (boneBillboardFlags != 0) {
        mat4 currentBoneMatCopy = currentBoneMat;
        vec3 scaleVector = vec3(
            length(currentBoneMat[0].xyz),
            length(currentBoneMat[1].xyz),
            length(currentBoneMat[2].xyz)
        );

        switch (boneBillboardFlags) {
            case 0x8: {
                if (isAnimated) {
                    vec4 xAxis = animatedMatrix[0];
                    currentBoneMat[0] = normalize(vec4(xAxis.y, xAxis.z, -xAxis.x, 0.0));

                    vec4 yAxis = animatedMatrix[1];
                    currentBoneMat[1] = normalize(vec4(yAxis.y, yAxis.z, -yAxis.x, 0.0));

                    vec4 zAxis = animatedMatrix[2];
                    currentBoneMat[2] = normalize(vec4(zAxis.y, zAxis.z, -zAxis.x, 0.0));
                } else {
                    currentBoneMat[0] = vec4(0, 0, -1, 0);
                    currentBoneMat[1] = vec4(1.0, 0, 0, 0); // isMirrored is dead code on CPU
                    currentBoneMat[2] = vec4(0, 1.0, 0, 0);
                }
                break;
            }
            case 0x10: {
                currentBoneMat[0] = normalize(currentBoneMat[0]);

                // CPU: newYAxis = vec4(M(1,0), -M(0,0), 0, 0); mathfu M(r,c) == slang m[c][r]
                vec4 newYAxis = vec4(
                    currentBoneMat[0][1],
                    -currentBoneMat[0][0],
                    0.0,
                    0.0
                );
                currentBoneMat[1] = normalize(newYAxis);
                currentBoneMat[2] = vec4(
                    cross(currentBoneMat[1].xyz, currentBoneMat[0].xyz),
                    0.0
                );
                break;
            }
            case 0x20: {
                float yAxisLen = length(currentBoneMat[1]);
                currentBoneMat[1] = currentBoneMat[1] * (1.0 / yAxisLen);

                // CPU: newXAxis = vec4(-M(1,1), M(0,1), 0, 0)
                vec4 newXAxis = vec4(
                    -currentBoneMat[1][1],
                    currentBoneMat[1][0],
                    0.0,
                    0.0
                );
                currentBoneMat[0] = normalize(newXAxis);
                currentBoneMat[2] = vec4(
                    cross(currentBoneMat[1].xyz, currentBoneMat[0].xyz),
                    0.0
                );
                break;
            }
            case 0x40: {
                currentBoneMat[2] = normalize(currentBoneMat[2]);
                vec4 newYAxis = vec4(
                    currentBoneMat[2].y,
                    -currentBoneMat[2].x,
                    0.0,
                    0.0
                );
                currentBoneMat[1] = normalize(newYAxis);
                currentBoneMat[0] = vec4(
                    cross(currentBoneMat[2].xyz, currentBoneMat[1].xyz),
                    0.0
                );
                break;
            }
            case 0x4000000: {
                vec4 pivotVec4 = vec4(bone.pivot, 1.0);
                vec4 transfVec = currentBoneMatCopy * pivotVec4;

                currentBoneMat[0] = normalize(vec4(-transfVec.x, -transfVec.y, -transfVec.z, 0.0));
                currentBoneMat[1] = normalize(vec4(
                    -currentBoneMat[0].z,
                    0.0,
                    0.0,
                    0.0));
                currentBoneMat[2] = vec4(
                    cross(currentBoneMat[1].xyz, currentBoneMat[0].xyz),
                    0.0
                );
                break;
            }
            default: break;
        }

        vec4 pivotVec4 = vec4(bone.pivot, 1.0);
        vec4 pivotVec3 = vec4(bone.pivot, 0.0);

        currentBoneMat[0] = currentBoneMat[0] * scaleVector.x;
        currentBoneMat[1] = currentBoneMat[1] * scaleVector.y;
        currentBoneMat[2] = currentBoneMat[2] * scaleVector.z;

        currentBoneMat[3] = (currentBoneMatCopy * pivotVec4) - (currentBoneMat * pivotVec3);
        currentBoneMat[3].w = 1.0;
    }

    return currentBoneMat;
}

// ---------------------------------------------------------------------------
// Per-frame particle emitter property evaluation
// (AnimationManager::calcParticleEmitters, animationManager.cpp:1326-1474)
// ---------------------------------------------------------------------------

float evalEmitterFloatTrack(in GpuM2AnimState state, in int trackIndex, in float defaultValue) {
    if (trackIndex < 0) return defaultValue;
    // CPU evaluates emitter/ribbon props with the OWN global-sequence bank
    // unconditionally (calcParticleEmitters/calcRibbonEmitters) — gsBank = 0
    return evalTrackBlendFloat(state, trackHeaders[state.trackHeaderOffset + trackIndex], defaultValue, 0,
                               seqDurations, globalLoops, seqSlices, trackTimestamps, trackValuesFloat);
}

[shader("compute")]
[numthreads(64, 1, 1)]
void main() {
    uint stateIndex = animDispatchList[gl_WorkGroupID.x];
    GpuM2AnimState state = animStates[stateIndex];

    // ---- Bones ----
    // Bank selected by the CURRENT animation's foundInParent (calcBoneMatrix:393-396)
    int boneBase = state.boneStaticOffset + (state.animFoundInParent != 0 ? state.ownBoneCount : 0);
    int boneCount = state.ownBoneCount;

    mat4 modelViewMat = matFromArray16(state.modelViewMat);
    mat4 invModelViewMat = matFromArray16(state.invModelViewMat);

    for (int boneIndex = int(gl_LocalInvocationIndex); boneIndex < boneCount; boneIndex += 64) {
        // Walk the parent chain up (root ... bone), then compose root->leaf so every
        // child sees its parent's final (post-fixup) matrix, exactly like the CPU's
        // memoized recursion.
        int chain[64]; // GPU_ANIM_MAX_BONE_CHAIN_DEPTH
        int chainLen = 0;
        int b = boneIndex;
        while (b >= 0 && chainLen < 64) {
            chain[chainLen++] = b;
            b = boneStatics[boneBase + b].parentBone;
        }

        mat4 mat = modelViewMat;
        for (int i = chainLen - 1; i >= 0; i--) {
            int bi = chain[i];
            GpuM2BoneStatic bone = boneStatics[boneBase + bi];
            bool isRoot = bone.parentBone < 0;
            mat = calcBoneMatrixGpu(state, bone, mat, modelViewMat, isRoot);
        }

        // CPU post-multiplies invModelView into every bone (animationManager.cpp:931-934)
        boneMatrixOut[state.boneOutOffset + boneIndex] = invModelViewMat * mat;
    }

    // ---- Ribbon per-frame props (AnimationManager::calcRibbonEmitters) ----
    for (int ribbonIdx = int(gl_LocalInvocationIndex); ribbonIdx < state.ribbonCount; ribbonIdx += 64) {
        GpuM2RibbonStatic rst = ribbonStatics[state.ribbonStaticsOffset + ribbonIdx];

        GpuM2RibbonFrameProps rp;

        vec4 color = vec4(1.0, 1.0, 1.0, 1.0);
        if (rst.colorTrack >= 0) {
            color = evalTrackBlendVec4(state, trackHeaders[state.trackHeaderOffset + rst.colorTrack],
                                       vec4(1.0, 1.0, 1.0, 1.0), false, 0 /* own GS bank, as on CPU */,
                                       seqDurations, globalLoops, seqSlices, trackTimestamps, trackValuesVec4);
        }
        rp.color[0] = color.x;
        rp.color[1] = color.y;
        rp.color[2] = color.z;

        rp.alpha = evalEmitterFloatTrack(state, rst.alphaTrack, 1.0);
        rp.above = evalEmitterFloatTrack(state, rst.heightAboveTrack, 1.0);
        rp.below = evalEmitterFloatTrack(state, rst.heightBelowTrack, 1.0);

        // SetTexSlot (the CPU's guard only skips recomputation; recomputing is idempotent)
        int texSlot = int(evalEmitterFloatTrack(state, rst.texSlotTrack, 0.0));
        int cols = max(rst.textureCols, 1);
        float minx = float(texSlot % cols) * rst.tmpDU + rst.texBoxMinX;
        float miny = float(texSlot / cols) * rst.tmpDV + rst.texBoxMinY;
        rp.texSlotBox[0] = minx;
        rp.texSlotBox[1] = miny;
        rp.texSlotBox[2] = minx + rst.tmpDU;
        rp.texSlotBox[3] = miny + rst.tmpDV;

        rp.dataEnabled = int(evalEmitterFloatTrack(state, rst.visibilityTrack, 1.0));

        ribbonProps[state.ribbonPropsOffset + ribbonIdx] = rp;
    }

    // ---- Particle emitter per-frame props ----
    for (int emitterIdx = int(gl_LocalInvocationIndex); emitterIdx < state.particleEmitterCount; emitterIdx += 64) {
        GpuM2ParticleStatic st = particleStatics[state.particleStaticsOffset + emitterIdx];

        int enabledIn = 1;
        if (st.enabledInTrack >= 0) {
            enabledIn = int(evalEmitterFloatTrack(state, st.enabledInTrack, 1.0));
        }

        GpuM2ParticleFrameProps props;
        props.isEnabled = enabledIn;

        if (enabledIn != 0) {
            props.emissionSpeed   = evalEmitterFloatTrack(state, st.emissionSpeedTrack, 0.0);
            props.speedVariation  = evalEmitterFloatTrack(state, st.speedVariationTrack, 0.0);
            props.verticalRange   = evalEmitterFloatTrack(state, st.verticalRangeTrack, 0.0);
            props.horizontalRange = evalEmitterFloatTrack(state, st.horizontalRangeTrack, 0.0);

            if (st.gravityIsCompressed != 0) {
                vec4 g = vec4(1.0, 1.0, 1.0, 0.0); // CPU defaultVector = (1,1,1)
                if (st.gravityTrack >= 0) {
                    g = evalTrackBlendVec4(state, trackHeaders[state.trackHeaderOffset + st.gravityTrack],
                                           vec4(1.0, 1.0, 1.0, 0.0), false, 0 /* own GS bank, as on CPU */,
                                           seqDurations, globalLoops, seqSlices, trackTimestamps, trackValuesVec4);
                }
                vec3ToArr(g.xyz, props.gravity);
            } else {
                float g = evalEmitterFloatTrack(state, st.gravityTrack, 0.0);
                vec3ToArr(vec3(0, 0, -g), props.gravity);
            }

            props.lifespan       = evalEmitterFloatTrack(state, st.lifespanTrack, 0.0);
            props.emissionRate   = evalEmitterFloatTrack(state, st.emissionRateTrack, 0.0);
            props.emissionAreaX  = evalEmitterFloatTrack(state, st.emissionAreaXTrack, 0.0);
            props.emissionAreaY  = evalEmitterFloatTrack(state, st.emissionAreaYTrack, 0.0);
            // With EXP2 the zSource is the static EXP2 value (set at init on CPU)
            props.zSource = (st.hasExp2 != 0) ? st.zSourceExp2
                                              : evalEmitterFloatTrack(state, st.zSourceTrack, 0.0);

            particleProps[state.particlePropsOffset + emitterIdx] = props;
        } else {
            // CPU evaluates props only when enabled; keep previous values otherwise.
            // isEnabled must still be written.
            GpuM2ParticleFrameProps prev = particleProps[state.particlePropsOffset + emitterIdx];
            prev.isEnabled = 0;
            particleProps[state.particlePropsOffset + emitterIdx] = prev;
        }
    }
}
