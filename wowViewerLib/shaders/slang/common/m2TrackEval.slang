//
// GPU evaluation of M2 animation tracks (M2Track<T>), mirroring the CPU code in
// engine/algorithms/animate.h and engine/managers/animationManager.cpp exactly.
//
// Data layout is packed at load time by buildM2GpuTrackPack() (M2GpuAnimData.h):
//   - one track-header per M2Track, one slice per (track, sequence)
//   - timestamps: flat uint ms values
//   - values: typed flat pools (vec4 for vec3/quat, float for scalars)
// Quaternions are stored as vec4(x,y,z,w), already converted from Quat16 and normalized.
//
// All buffers use ScalarDataLayout so C++ struct layouts match 1:1.
//
#ifndef M2_TRACK_EVAL_SLANG
#define M2_TRACK_EVAL_SLANG

import glsl;

struct GpuM2BoneStatic {
    float3 pivot;
    int parentBone;
    uint flagsRaw;
    int transTrack;
    int rotTrack;
    int scaleTrack;
};

struct GpuM2TrackHeader {
    int interpolationType;
    int globalSequence;   // -1 = none
    int seqSliceOffset;
    int seqSliceCount;
};

struct GpuM2SeqSlice {
    uint timeOffset;
    uint valueOffset;
    uint elemCount;
    uint pad;
};

#define GPU_ANIM_MAX_GLOBAL_SEQUENCES 32

struct GpuM2AnimState {
    float modelViewMat[16];
    float invModelViewMat[16];

    int animIndex;
    float animTime;
    int animFoundInParent;
    int nextAnimIndex;
    float nextAnimTime;
    int nextAnimFoundInParent;
    float blendFactor;

    int boneStaticOffset;
    int ownBoneCount;
    int parentBoneCount;
    int boneOutOffset;
    int trackHeaderOffset;
    int seqSliceOffset;
    int timestampOffset;
    int valuesVec4Offset;
    int valuesFloatOffset;
    int seqDurationOffset;
    int ownSeqCount;
    int parentSeqCount;
    int globalLoopOffset;
    int ownGsCount;
    int parentGsCount;

    int particleStaticsOffset;
    int particlePropsOffset;
    int particleEmitterCount;
    int partTimesOffset;
    int ribbonStaticsOffset;
    int ribbonPropsOffset;
    int ribbonCount;
    int pad3[3];

    float gsTimes[GPU_ANIM_MAX_GLOBAL_SEQUENCES];
    float parentGsTimes[GPU_ANIM_MAX_GLOBAL_SEQUENCES];
};

mat4 matFromArray16(in float a[16]) {
    // CPU writes mathfu::mat4 memory order (column-major); slang mat4 ctor fills
    // column-major too, so the matrix is reconstructed 1:1.
    return mat4(a[0], a[1], a[2], a[3],
                a[4], a[5], a[6], a[7],
                a[8], a[9], a[10], a[11],
                a[12], a[13], a[14], a[15]);
}

// ---------------------------------------------------------------------------
// mathfu-compatible quaternion helpers (quat stored as vec4(x,y,z,w))
// ---------------------------------------------------------------------------

vec4 quatMul(in vec4 a, in vec4 b) {
    // mathfu: w = aw*bw - dot(av,bv); v = aw*bv + bw*av + cross(av,bv)
    float w = a.w * b.w - dot(a.xyz, b.xyz);
    float3 v = a.w * b.xyz + b.w * a.xyz + cross(a.xyz, b.xyz);
    return vec4(v, w);
}

// mathfu operator*(T s): angle scaling via ToAngleAxis/FromAngleAxis
vec4 quatScaleAngle(in vec4 qIn, in float t) {
    vec4 q = (qIn.w > 0.0) ? qIn : -qIn;
    float axisLength = length(q.xyz);
    float3 axis;
    if (axisLength == 0.0) {
        axis = float3(1.0, 0.0, 0.0);
    } else {
        axis = q.xyz / axisLength;
    }
    float angle = 2.0 * atan2(axisLength, q.w) * t;
    float halfAngle = 0.5 * angle;
    float3 naxis = normalize(axis); // mathfu normalizes again in FromAngleAxis
    return vec4(naxis * sin(halfAngle), cos(halfAngle));
}

// mathfu::quat::Slerp
vec4 quatSlerp(in vec4 a, in vec4 b, in float t) {
    float d = a.w * b.w + dot(a.xyz, b.xyz);
    if (d > 0.9999) {
        return normalize(mix(a, b, t));
    }
    // q1 * ((q1^-1 * q2) * t), q1^-1 = conjugate
    vec4 aInv = vec4(-a.xyz, a.w);
    vec4 tmp = quatScaleAngle(quatMul(aInv, b), t);
    return quatMul(a, tmp);
}

// mathfu::quat::ToMatrix4 — scalar order matches the column-major ctor 1:1
mat4 quatToMat4(in vec4 q) {
    float x = q.x, y = q.y, z = q.z, s = q.w;
    float x2 = x * x, y2 = y * y, z2 = z * z;
    float sx = s * x, sy = s * y, sz = s * z;
    float xz = x * z, yz = y * z, xy = x * y;
    return mat4(1.0 - 2.0 * (y2 + z2), 2.0 * (xy + sz),       2.0 * (xz - sy),       0.0,
                2.0 * (xy - sz),       1.0 - 2.0 * (x2 + z2), 2.0 * (sx + yz),       0.0,
                2.0 * (sy + xz),       2.0 * (yz - sx),       1.0 - 2.0 * (x2 + y2), 0.0,
                0.0,                   0.0,                   0.0,                   1.0);
}

// ---------------------------------------------------------------------------
// Track evaluation (animate.h :: findTimeIndex / animateTrack)
// ---------------------------------------------------------------------------

int findTimeIndexGpu(uint currTime,
                     in StructuredBuffer<uint, ScalarDataLayout> timestamps,
                     in uint timeOffset, in uint elemCount) {
    int len = int(elemCount);
    if (len > 1) {
        if (currTime > timestamps[timeOffset + uint(len) - 1u]) return len - 1;
        // std::lower_bound over [0, len): first index with timestamps[i] >= currTime
        int lo = 0, hi = len;
        while (lo < hi) {
            int mid = (lo + hi) >> 1;
            if (timestamps[timeOffset + uint(mid)] < currTime) lo = mid + 1;
            else hi = mid;
        }
        int idx = lo;
        if (idx != 0) idx -= 1;
        return idx;
    } else if (len == 1) {
        return 0;
    }
    return -1;
}

// Bank-aware accessors over the per-object state
float seqDurationOf(in GpuM2AnimState state, int bankParent, int animIndex,
                    in StructuredBuffer<uint, ScalarDataLayout> seqDurations) {
    int base = state.seqDurationOffset + (bankParent != 0 ? state.ownSeqCount : 0);
    return float(seqDurations[base + animIndex]);
}

// Bank-aware accessors over the per-object state.
// NOTE: on the CPU, ONLY bone tracks (calcBoneMatrix, animationManager.cpp:393-396)
// select the global-sequence bank by the current animation's foundInParent. All
// non-bone tracks (particle/ribbon emitter props, colors, transparencies...) are
// always evaluated with the OWN bank's global-sequence times/loops
// (calcParticleEmitters/calcRibbonEmitters pass this->globalSequenceTimes
// unconditionally). Callers therefore pass gsBank = state.animFoundInParent for
// bones and gsBank = 0 for everything else.
float gsTimeOf(in GpuM2AnimState state, int bankParent, int gsIndex) {
    if (bankParent != 0)
        return state.parentGsTimes[gsIndex];
    return state.gsTimes[gsIndex];
}

uint gsLoopOf(in GpuM2AnimState state, int bankParent, int gsIndex,
              in StructuredBuffer<uint, ScalarDataLayout> globalLoops) {
    int base = state.globalLoopOffset + (bankParent != 0 ? state.ownGsCount : 0);
    return globalLoops[base + gsIndex];
}

// Core single-animation track evaluation for the vec4 pool.
// Returns defaultValue when the slice is empty/unloaded; replicates the CPU quirk
// of returning defaultValue for hermite interpolation types (2/3).
// isQuat: rotation tracks interpolate keyframes with mathfu::quat::Slerp
// (lerpHelper<mathfu::quat>), everything else uses plain Lerp.
vec4 evalM2TrackVec4(in GpuM2TrackHeader header,
                     in int animIndex, in float currTime, in float maxTime,
                     in StructuredBuffer<GpuM2SeqSlice, ScalarDataLayout> seqSlices,
                     in int seqSliceOffsetBase,
                     in StructuredBuffer<uint, ScalarDataLayout> timestamps,
                     in int timestampOffsetBase,
                     in StructuredBuffer<vec4, ScalarDataLayout> valuesVec4,
                     in int valuesVec4OffsetBase,
                     in vec4 defaultValue,
                     in bool isQuat) {
    int sliceCount = header.seqSliceCount;
    if (sliceCount == 0) return defaultValue;
    int ai = animIndex;
    if (sliceCount <= ai) ai = 0;

    GpuM2SeqSlice slice = seqSlices[seqSliceOffsetBase + header.seqSliceOffset + ai];
    if (slice.elemCount == 0) return defaultValue;

    uint timeBase = uint(timestampOffsetBase) + slice.timeOffset;
    uint valueBase = uint(valuesVec4OffsetBase) + slice.valueOffset;

    int timeIndex = (maxTime != 0.0)
        ? findTimeIndexGpu(uint(currTime), timestamps, timeBase, slice.elemCount)
        : 0;

    if (timeIndex == int(slice.elemCount) - 1) {
        return valuesVec4[valueBase + uint(timeIndex)];
    } else if (timeIndex >= 0) {
        vec4 value1 = valuesVec4[valueBase + uint(timeIndex)];
        vec4 value2 = valuesVec4[valueBase + uint(timeIndex) + 1u];

        float time1 = float(timestamps[timeBase + uint(timeIndex)]);
        float time2 = float(timestamps[timeBase + uint(timeIndex) + 1u]);

        if (header.interpolationType == 0) {
            return value1;
        } else if (header.interpolationType == 1) {
            float t = (currTime - time1) / (time2 - time1);
            return isQuat ? quatSlerp(value1, value2, t)
                          : mix(value1, value2, t);
        }
        // interp 2/3: CPU animateTrack falls through to defaultValue
        return defaultValue;
    }
    return valuesVec4[valueBase];
}

// Scalar-pool variant (same semantics)
float evalM2TrackFloat(in GpuM2TrackHeader header,
                       in int animIndex, in float currTime, in float maxTime,
                       in StructuredBuffer<GpuM2SeqSlice, ScalarDataLayout> seqSlices,
                       in int seqSliceOffsetBase,
                       in StructuredBuffer<uint, ScalarDataLayout> timestamps,
                       in int timestampOffsetBase,
                       in StructuredBuffer<float, ScalarDataLayout> valuesFloat,
                       in int valuesFloatOffsetBase,
                       in float defaultValue) {
    int sliceCount = header.seqSliceCount;
    if (sliceCount == 0) return defaultValue;
    int ai = animIndex;
    if (sliceCount <= ai) ai = 0;

    GpuM2SeqSlice slice = seqSlices[seqSliceOffsetBase + header.seqSliceOffset + ai];
    if (slice.elemCount == 0) return defaultValue;

    uint timeBase = uint(timestampOffsetBase) + slice.timeOffset;
    uint valueBase = uint(valuesFloatOffsetBase) + slice.valueOffset;

    int timeIndex = (maxTime != 0.0)
        ? findTimeIndexGpu(uint(currTime), timestamps, timeBase, slice.elemCount)
        : 0;

    if (timeIndex == int(slice.elemCount) - 1) {
        return valuesFloat[valueBase + uint(timeIndex)];
    } else if (timeIndex >= 0) {
        float value1 = valuesFloat[valueBase + uint(timeIndex)];
        float value2 = valuesFloat[valueBase + uint(timeIndex) + 1u];

        float time1 = float(timestamps[timeBase + uint(timeIndex)]);
        float time2 = float(timestamps[timeBase + uint(timeIndex) + 1u]);

        if (header.interpolationType == 0) {
            return value1;
        } else if (header.interpolationType == 1) {
            return mix(value1, value2, (currTime - time1) / (time2 - time1));
        }
        return defaultValue;
    }
    return valuesFloat[valueBase];
}

// ---------------------------------------------------------------------------
// animateTrackWithBlend equivalents
// ---------------------------------------------------------------------------

vec4 evalTrackBlendVec4(in GpuM2AnimState state,
                        in GpuM2TrackHeader header,
                        in vec4 defaultValue,
                        bool isQuat,
                        in int gsBank,
                        in StructuredBuffer<uint, ScalarDataLayout> seqDurations,
                        in StructuredBuffer<uint, ScalarDataLayout> globalLoops,
                        in StructuredBuffer<GpuM2SeqSlice, ScalarDataLayout> seqSlices,
                        in StructuredBuffer<uint, ScalarDataLayout> timestamps,
                        in StructuredBuffer<vec4, ScalarDataLayout> valuesVec4) {
    float currTime = state.animTime;
    float maxTime = seqDurationOf(state, state.animFoundInParent, state.animIndex, seqDurations);
    if (header.globalSequence >= 0) {
        currTime = gsTimeOf(state, gsBank, header.globalSequence);
        maxTime = float(gsLoopOf(state, gsBank, header.globalSequence, globalLoops));
    }

    vec4 result = evalM2TrackVec4(header, state.animIndex, currTime, maxTime,
                                  seqSlices, state.seqSliceOffset,
                                  timestamps, state.timestampOffset,
                                  valuesVec4, state.valuesVec4Offset,
                                  defaultValue, isQuat);

    if (state.nextAnimIndex > -1 && state.blendFactor < 0.999) {
        float nextTime = state.nextAnimTime;
        float nextMaxTime = seqDurationOf(state, state.nextAnimFoundInParent, state.nextAnimIndex, seqDurations);
        if (header.globalSequence >= 0) {
            // CPU: the same (gsBank-selected) global sequence times are used for the blend eval
            nextTime = gsTimeOf(state, gsBank, header.globalSequence);
            nextMaxTime = float(gsLoopOf(state, gsBank, header.globalSequence, globalLoops));
        }

        vec4 result1 = evalM2TrackVec4(header, state.nextAnimIndex, nextTime, nextMaxTime,
                                       seqSlices, state.seqSliceOffset,
                                       timestamps, state.timestampOffset,
                                       valuesVec4, state.valuesVec4Offset,
                                       defaultValue, isQuat);
        // CPU: lerpHelper(next, current, blendFactor) — slerp for quats
        result = isQuat ? quatSlerp(result1, result, state.blendFactor)
                        : mix(result1, result, state.blendFactor);
    }
    return result;
}

float evalTrackBlendFloat(in GpuM2AnimState state,
                          in GpuM2TrackHeader header,
                          in float defaultValue,
                          in int gsBank,
                          in StructuredBuffer<uint, ScalarDataLayout> seqDurations,
                          in StructuredBuffer<uint, ScalarDataLayout> globalLoops,
                          in StructuredBuffer<GpuM2SeqSlice, ScalarDataLayout> seqSlices,
                          in StructuredBuffer<uint, ScalarDataLayout> timestamps,
                          in StructuredBuffer<float, ScalarDataLayout> valuesFloat) {
    float currTime = state.animTime;
    float maxTime = seqDurationOf(state, state.animFoundInParent, state.animIndex, seqDurations);
    if (header.globalSequence >= 0) {
        currTime = gsTimeOf(state, gsBank, header.globalSequence);
        maxTime = float(gsLoopOf(state, gsBank, header.globalSequence, globalLoops));
    }

    float result = evalM2TrackFloat(header, state.animIndex, currTime, maxTime,
                                    seqSlices, state.seqSliceOffset,
                                    timestamps, state.timestampOffset,
                                    valuesFloat, state.valuesFloatOffset,
                                    defaultValue);

    if (state.nextAnimIndex > -1 && state.blendFactor < 0.999) {
        float nextTime = state.nextAnimTime;
        float nextMaxTime = seqDurationOf(state, state.nextAnimFoundInParent, state.nextAnimIndex, seqDurations);
        if (header.globalSequence >= 0) {
            nextTime = gsTimeOf(state, gsBank, header.globalSequence);
            nextMaxTime = float(gsLoopOf(state, gsBank, header.globalSequence, globalLoops));
        }

        float result1 = evalM2TrackFloat(header, state.nextAnimIndex, nextTime, nextMaxTime,
                                         seqSlices, state.seqSliceOffset,
                                         timestamps, state.timestampOffset,
                                         valuesFloat, state.valuesFloatOffset,
                                         defaultValue);
        result = mix(result1, result, state.blendFactor);
    }
    return result;
}

// ---------------------------------------------------------------------------
// M2PartTrack evaluation (per-particle-age tracks; animate.h :: animatePartTrack)
// Times are raw fixed16 values in the partTimes pool; age is a normalized [0..1]
// float. All slice offsets are relative to the object's chunk; partTimesOffsetBase
// shifts them into the global pool (same for the value pools' bases).
// ---------------------------------------------------------------------------

// Age -> fixed16 like convertHelper<animTime_t, fixed16> (floor(age * 32768))
uint ageToFixed16(float age) {
    return uint(floor(age * 32768.0));
}

int findTimeIndexPart(uint ageFixed,
                      in StructuredBuffer<uint, ScalarDataLayout> partTimes,
                      in uint timeOffset, in uint elemCount) {
    int len = int(elemCount);
    if (len > 1) {
        if (ageFixed > partTimes[timeOffset + uint(len) - 1u]) return len - 1;
        int lo = 0, hi = len;
        while (lo < hi) {
            int mid = (lo + hi) >> 1;
            if (partTimes[timeOffset + uint(mid)] < ageFixed) lo = mid + 1;
            else hi = mid;
        }
        int idx = lo;
        if (idx != 0) idx -= 1;
        return idx;
    } else if (len == 1) {
        return 0;
    }
    return -1;
}

float partTimeToFloat(uint fixedTs) {
    return float(fixedTs) / 32768.0;
}

vec4 evalM2PartTrackVec4(in GpuM2SeqSlice slice,
                         in float age,
                         in StructuredBuffer<uint, ScalarDataLayout> partTimes,
                         in int partTimesOffsetBase,
                         in StructuredBuffer<vec4, ScalarDataLayout> valuesVec4,
                         in int valuesVec4OffsetBase,
                         in vec4 defaultValue) {
    if (slice.elemCount == 0) return defaultValue;

    uint ageFixed = ageToFixed16(age);
    uint timeBase = uint(partTimesOffsetBase) + slice.timeOffset;
    int timeIndex = findTimeIndexPart(ageFixed, partTimes, timeBase, slice.elemCount);

    if (timeIndex == int(slice.elemCount) - 1) {
        return valuesVec4[uint(valuesVec4OffsetBase) + slice.valueOffset + uint(timeIndex)];
    } else if (timeIndex >= 0) {
        vec4 value1 = valuesVec4[uint(valuesVec4OffsetBase) + slice.valueOffset + uint(timeIndex)];
        vec4 value2 = valuesVec4[uint(valuesVec4OffsetBase) + slice.valueOffset + uint(timeIndex) + 1u];

        float time1 = partTimeToFloat(partTimes[timeBase + uint(timeIndex)]);
        float time2 = partTimeToFloat(partTimes[timeBase + uint(timeIndex) + 1u]);

        return mix(value1, value2, (age - time1) / (time2 - time1));
    }
    return valuesVec4[uint(valuesVec4OffsetBase) + slice.valueOffset];
}

float evalM2PartTrackFloat(in GpuM2SeqSlice slice,
                           in float age,
                           in StructuredBuffer<uint, ScalarDataLayout> partTimes,
                           in int partTimesOffsetBase,
                           in StructuredBuffer<float, ScalarDataLayout> valuesFloat,
                           in int valuesFloatOffsetBase,
                           in float defaultValue) {
    if (slice.elemCount == 0) return defaultValue;

    uint ageFixed = ageToFixed16(age);
    uint timeBase = uint(partTimesOffsetBase) + slice.timeOffset;
    int timeIndex = findTimeIndexPart(ageFixed, partTimes, timeBase, slice.elemCount);

    if (timeIndex == int(slice.elemCount) - 1) {
        return valuesFloat[uint(valuesFloatOffsetBase) + slice.valueOffset + uint(timeIndex)];
    } else if (timeIndex >= 0) {
        float value1 = valuesFloat[uint(valuesFloatOffsetBase) + slice.valueOffset + uint(timeIndex)];
        float value2 = valuesFloat[uint(valuesFloatOffsetBase) + slice.valueOffset + uint(timeIndex) + 1u];

        float time1 = partTimeToFloat(partTimes[timeBase + uint(timeIndex)]);
        float time2 = partTimeToFloat(partTimes[timeBase + uint(timeIndex) + 1u]);

        return mix(value1, value2, (age - time1) / (time2 - time1));
    }
    return valuesFloat[uint(valuesFloatOffsetBase) + slice.valueOffset];
}

// uint16-cell variant: CPU lerps then truncates to uint16
int evalM2PartTrackCell(in GpuM2SeqSlice slice,
                        in float age,
                        in StructuredBuffer<uint, ScalarDataLayout> partTimes,
                        in int partTimesOffsetBase,
                        in StructuredBuffer<float, ScalarDataLayout> valuesFloat,
                        in int valuesFloatOffsetBase,
                        in int defaultValue) {
    float v = evalM2PartTrackFloat(slice, age, partTimes, partTimesOffsetBase, valuesFloat, valuesFloatOffsetBase, float(defaultValue));
    return int(v); // truncation, matching (uint16_t)(v1*(1-p) + v2*p)
}

#endif //M2_TRACK_EVAL_SLANG
