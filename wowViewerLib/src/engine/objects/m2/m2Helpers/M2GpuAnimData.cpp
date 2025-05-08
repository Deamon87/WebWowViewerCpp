//
// Load-time packing of M2 bone animation data into GPU-ready arrays.
// Mirrors the CPU evaluation semantics from engine/algorithms/animate.h exactly:
//  - sequences without flag 0x20 are lazy (.anim files, not loaded at pack time):
//    their slices are packed EMPTY (elemCount=0 -> default value). The pack stays
//    valid; when the sequence streams in (loadLowPriority sets flag 0x20), the
//    shared track set rebuilds the pack and re-uploads (IM2GpuTrackSet::syncTrackData);
//  - quaternion values are converted from Quat16 and normalized at pack time.
//

#include "M2GpuAnimData.h"
#include "CBoneMasterData.h"
#include "../../../algorithms/animate.h"

namespace {

enum class TrackValueKind { Vec4, Float };

// Packs one M2Track into the pack's flat arrays. Returns the track-header index,
// or -1 when the track has no value arrays at all (caller then skips the factor,
// matching the CPU's `track.values.size > 0` checks).
// Sequences that are not resident (lazy .anim, flag 0x20 unset) get an empty slice
// (elemCount=0); the CPU never evaluates unloaded sequences either (the sequencing
// gate in updateSequencing), so this keeps GPU/CPU parity.
template<typename T, typename Convert>
int32_t packM2Track(const M2Track<T> &track,
                    M2GpuTrackPack &pack,
                    const M2Array<M2Sequence> *sequences,
                    TrackValueKind kind,
                    Convert convert /* T -> appended into valuesVec4/valuesFloat */) {
    if (track.values.size <= 0 || track.timestamps.size <= 0) {
        return -1;
    }

    GpuM2TrackHeader header;
    header.interpolationType = track.interpolation_type;
    header.globalSequence = track.global_sequence; // int16 -> int32, -1 stays -1
    header.seqSliceOffset = (int32_t)pack.seqSlices.size();
    header.seqSliceCount = track.timestamps.size;

    for (int32_t seqIdx = 0; seqIdx < track.timestamps.size; seqIdx++) {
        GpuM2SeqSlice slice;
        slice.timeOffset = (uint32_t)pack.timestamps.size();
        slice.valueOffset = (kind == TrackValueKind::Vec4)
                            ? (uint32_t)(pack.valuesVec4.size() / 4)
                            : (uint32_t)pack.valuesFloat.size();
        slice.elemCount = 0;
        slice.pad = 0;

        const bool seqLoaded = (sequences != nullptr)
                               && (sequences->size > seqIdx)
                               && ((sequences->getElement(seqIdx)->flags & 0x20) != 0);
        if (!seqLoaded) {
            // Lazy .anim data not in memory; inner M2Arrays of this sequence are
            // uninitialized file offsets and must not be dereferenced. Pack an
            // empty slice; a later rebuild (updateTrackData) fills it in.
            pack.seqSlices.push_back(slice);
            continue;
        }

        auto *times = track.timestamps.getElement(seqIdx);
        auto *values = track.values.getElement(seqIdx);
        int32_t count = 0;
        if (times != nullptr && values != nullptr) {
            count = (int32_t)std::min(times->size, values->size);
        }

        for (int32_t i = 0; i < count; i++) {
            pack.timestamps.push_back(*times->getElement(i));
            convert(*values->getElement(i), pack);
        }
        slice.elemCount = (uint32_t)count;

        pack.seqSlices.push_back(slice);
    }

    pack.trackHeaders.push_back(header);
    return (int32_t)pack.trackHeaders.size() - 1;
}

bool packBank(const SkelData *skelData,
              M2GpuTrackPack &pack,
              bool isParentBank,
              int32_t &boneCountOut) {
    boneCountOut = 0;
    if (skelData == nullptr) return true;

    bool packValid = true;

    // Sequences (durations); also count resident ones (flag 0x20) so the object
    // can detect .anim streaming completion by comparing counts later
    auto *sequences = skelData->m_sequences;
    if (sequences != nullptr) {
        int32_t loadedCount = 0;
        for (int32_t i = 0; i < sequences->size; i++) {
            auto *seq = sequences->getElement(i);
            pack.seqDurations.push_back(seq->duration);
            if ((seq->flags & 0x20) != 0) loadedCount++;
        }
        if (isParentBank) {
            pack.parentSeqCount = sequences->size;
            pack.loadedSeqCountParent = loadedCount;
        } else {
            pack.ownSeqCount = sequences->size;
            pack.loadedSeqCountOwn = loadedCount;
        }
    }

    // Global loops (timestamps)
    auto *globalLoops = skelData->m_globalSequences;
    if (globalLoops != nullptr) {
        if (globalLoops->size > GPU_ANIM_MAX_GLOBAL_SEQUENCES) return false;
        for (int32_t i = 0; i < globalLoops->size; i++) {
            pack.globalLoops.push_back(globalLoops->getElement(i)->timestamp);
        }
        if (isParentBank) pack.parentGsCount = globalLoops->size;
        else pack.ownGsCount = globalLoops->size;
    }

    // Bones
    auto *bones = skelData->m_m2CompBones;
    if (bones != nullptr && bones->size > 0) {
        boneCountOut = bones->size;

        // Bone chain depth check (GPU replays parent chains per thread)
        for (int32_t i = 0; i < bones->size; i++) {
            int32_t depth = 0;
            int32_t b = i;
            while (depth <= GPU_ANIM_MAX_BONE_CHAIN_DEPTH) {
                int32_t parent = bones->getElement(b)->parent_bone;
                if (parent < 0) break;
                if (parent >= bones->size) return false; // corrupted parent link
                b = parent;
                depth++;
            }
            if (depth > GPU_ANIM_MAX_BONE_CHAIN_DEPTH) return false;
        }

        for (int32_t i = 0; i < bones->size; i++) {
            M2CompBone *bone = bones->getElement(i);

            GpuM2BoneStatic gpuBone;
            gpuBone.pivot[0] = bone->pivot.x;
            gpuBone.pivot[1] = bone->pivot.y;
            gpuBone.pivot[2] = bone->pivot.z;
            gpuBone.parentBone = bone->parent_bone;
            gpuBone.flagsRaw = bone->flags_raw;

            gpuBone.transTrack = packM2Track<C3Vector>(bone->translation, pack, sequences,
                                                       TrackValueKind::Vec4,
                [](const C3Vector &v, M2GpuTrackPack &p) {
                    p.valuesVec4.insert(p.valuesVec4.end(), {v.x, v.y, v.z, 0.0f});
                });

            gpuBone.rotTrack = packM2Track<Quat16>(bone->rotation, pack, sequences,
                                                   TrackValueKind::Vec4,
                [](const Quat16 &v, M2GpuTrackPack &p) {
                    mathfu::quat q = convertHelper<Quat16, mathfu::quat>(v);
                    p.valuesVec4.insert(p.valuesVec4.end(), {q[1], q[2], q[3], q[0]}); // x,y,z,w
                });

            gpuBone.scaleTrack = packM2Track<C3Vector>(bone->scaling, pack, sequences,
                                                       TrackValueKind::Vec4,
                [](const C3Vector &v, M2GpuTrackPack &p) {
                    p.valuesVec4.insert(p.valuesVec4.end(), {v.x, v.y, v.z, 0.0f});
                });

            pack.boneStatics.push_back(gpuBone);
        }
    }

    return packValid;
}

// Packs one M2PartTrack (per-particle-age track) into partTimes + a value pool.
// Timestamps are stored as raw fixed16 values (the shader searches in fixed16 space,
// matching animatePartTrack's convertHelper<animTime_t,fixed16> age truncation).
template<typename T, typename Convert>
GpuM2SeqSlice packM2PartTrack(const M2PartTrack<T> &track,
                              M2GpuTrackPack &pack,
                              TrackValueKind kind,
                              Convert convert) {
    GpuM2SeqSlice slice;
    slice.timeOffset = (uint32_t)pack.partTimes.size();
    slice.valueOffset = (kind == TrackValueKind::Vec4)
                        ? (uint32_t)(pack.valuesVec4.size() / 4)
                        : (uint32_t)pack.valuesFloat.size();
    slice.elemCount = 0;
    slice.pad = 0;

    int32_t count = (int32_t)std::min(track.timestamps.size, track.values.size);
    for (int32_t i = 0; i < count; i++) {
        pack.partTimes.push_back((uint32_t)(uint16_t)*track.timestamps.getElement(i));
        convert(*track.values.getElement(i), pack);
    }
    slice.elemCount = (uint32_t)count;
    return slice;
}

static float paramXTransformCpp(uint32_t x) {
    return (((float)(x & 0x1F)) / 32.0f) + (float)(x >> 5);
}

// Max of a float track's values across all loaded sequences (for capacity estimation)
float maxTrackValue(const M2Track<float> &track) {
    float m = 0.0f;
    for (int32_t s = 0; s < track.values.size; s++) {
        auto *vals = track.values.getElement(s);
        if (vals == nullptr) continue;
        for (int32_t i = 0; i < vals->size; i++) {
            m = std::max(m, *vals->getElement(i));
        }
    }
    return m;
}

bool packParticleEmitters(M2GpuTrackPack &pack,
                          const M2Data *m2Data,
                          const EXP2 *exp2,
                          const M2Array<M2Sequence> *sequences) {
    bool packValid = true;

    auto &emitters = m2Data->particle_emitters;
    for (int32_t i = 0; i < emitters.size; i++) {
        M2Particle *pe = emitters.getElement(i);
        const auto &old = pe->old;

        GpuM2ParticleStatic st;
        std::memset(&st, 0, sizeof(st));

        st.flags = old.flags;
        st.bone = old.bone;
        st.position[0] = old.Position.x;
        st.position[1] = old.Position.y;
        st.position[2] = old.Position.z;
        st.emitterType = old.emitterType;
        st.blendingType = old.blendingType;
        st.particleColorIndex = old.particleColorIndex;

        st.lifespanVary = old.lifespanVary;
        st.emissionRateVary = old.emissionRateVary;
        st.scaleVary[0] = old.scaleVary.x;
        st.scaleVary[1] = old.scaleVary.y;

        st.tailLength = old.tailLength;
        st.twinkleSpeed = old.TwinkleSpeed;
        st.twinklePercent = old.TwinklePercent;
        st.twinkleScaleMin = old.twinkleScale.min;
        st.twinkleScaleMax = old.twinkleScale.max;
        st.burstMultiplier = old.BurstMultiplier;
        st.drag = old.drag;
        st.baseSpin = old.baseSpin;
        st.baseSpinVary = old.baseSpinVary;
        st.spin = old.Spin;
        st.spinVary = old.spinVary;

        st.windVector[0] = old.WindVector.x;
        st.windVector[1] = old.WindVector.y;
        st.windVector[2] = old.WindVector.z;
        st.windTime = old.WindTime;

        st.followSpeed1 = old.followSpeed1;
        st.followScale1 = old.followScale1;
        st.followSpeed2 = old.followSpeed2;
        st.followScale2 = old.followScale2;
        const float followDen = old.followSpeed2 - old.followSpeed1;
        if (followDen != 0.0f) {
            st.followMult = (old.followScale2 - old.followScale1) / followDen;
            st.followBase = old.followScale1 - old.followSpeed1 * st.followMult;
        } else {
            st.followMult = 0.0f;
            st.followBase = 0.0f;
        }

        // Texture cell grid math (mirrors the ParticleEmitter constructor)
        uint16_t cols = old.textureDimensions_columns > 0 ? old.textureDimensions_columns : 1;
        uint16_t rows = old.textureDimensions_rows > 0 ? old.textureDimensions_rows : 1;
        st.textureIndexMask = cols * rows - 1;
        int colsVal = cols, colBits = -1;
        do { ++colBits; colsVal >>= 1; } while (colsVal);
        st.textureColBits = colBits;
        st.textureColMask = cols - 1;
        st.texScaleX = 1.0f / (float)cols;
        st.texScaleY = 1.0f / (float)rows;

        // m_particleType (mirrors the ParticleEmitter constructor)
        if ((old.flags & 0x10100000) == 0) {
            st.particleType = 0;
        } else {
            const bool isMultitex = (0 != (1 & (old.flags >> 0x1c)));
            st.particleType = isMultitex ? 2 : 3;
        }

        // Multi-texture params (decoded exactly like MathHelper::fp69ToFloat)
        auto decodeV69 = [](const vector_2fp_6_9 &v, float out2[2]) {
            auto fp69 = [](uint16_t x) -> float {
                float result = (((x & 0x1FF) * 0.001953125f) + (float)(x >> 9));
                if ((x & 0x8000) != 0) {
                    result *= -1.0f;
                }
                return result;
            };
            out2[0] = fp69(v.x);
            out2[1] = fp69(v.y);
        };
        decodeV69(pe->multiTextureParam0[0], &st.multiTextureParam0[0]);
        decodeV69(pe->multiTextureParam0[1], &st.multiTextureParam0[2]);
        decodeV69(pe->multiTextureParam1[0], &st.multiTextureParam1[0]);
        decodeV69(pe->multiTextureParam1[1], &st.multiTextureParam1[2]);
        st.multiTextureParamX0 = paramXTransformCpp(old.multiTextureParamX[0]);
        st.multiTextureParamX1 = paramXTransformCpp(old.multiTextureParamX[1]);

        const Exp2Record *exp2Rec = nullptr;
        if (exp2 != nullptr) {
            exp2Rec = exp2->content.getElement(i);
        }
        st.hasExp2 = (exp2Rec != nullptr) ? 1 : 0;
        st.zSourceExp2 = exp2Rec ? exp2Rec->zSource : 0.0f;

        // Per-frame tracks
        auto packFloatTrack = [&](const M2Track<float> &track) -> int32_t {
            return packM2Track<float>(track, pack, sequences, TrackValueKind::Float,
                [](const float &v, M2GpuTrackPack &p) { p.valuesFloat.push_back(v); });
        };
        auto packUcharTrack = [&](const M2Track<unsigned char> &track) -> int32_t {
            return packM2Track<unsigned char>(track, pack, sequences, TrackValueKind::Float,
                [](const unsigned char &v, M2GpuTrackPack &p) { p.valuesFloat.push_back((float)v); });
        };

        st.emissionSpeedTrack = packFloatTrack(old.emissionSpeed);
        st.speedVariationTrack = packFloatTrack(old.speedVariation);
        st.verticalRangeTrack = packFloatTrack(old.verticalRange);
        st.horizontalRangeTrack = packFloatTrack(old.horizontalRange);

        if (old.flags & 0x800000) {
            // Compressed gravity track -> vec4 pool (converted at pack time)
            st.gravityIsCompressed = 1;
            st.gravityTrack = packM2Track<CompressedParticleGravity>(old.gravityCompr, pack, sequences,
                TrackValueKind::Vec4,
                [](const CompressedParticleGravity &v, M2GpuTrackPack &p) {
                    mathfu::vec3 g = convertHelper<CompressedParticleGravity, mathfu::vec3>(v);
                    p.valuesVec4.insert(p.valuesVec4.end(), {g.x, g.y, g.z, 0.0f});
                });
        } else {
            st.gravityIsCompressed = 0;
            st.gravityTrack = packFloatTrack(old.gravity);
        }

        st.lifespanTrack = packFloatTrack(old.lifespan);
        st.emissionRateTrack = packFloatTrack(old.emissionRate);
        st.emissionAreaXTrack = packFloatTrack(old.emissionAreaLength);
        st.emissionAreaYTrack = packFloatTrack(old.emissionAreaWidth);
        st.zSourceTrack = (exp2Rec == nullptr) ? packFloatTrack(old.zSource) : -1;
        st.enabledInTrack = packUcharTrack(old.enabledIn);

        // Per-age tracks
        st.colorTrackSlice = packM2PartTrack<C3Vector>(old.colorTrack, pack, TrackValueKind::Vec4,
            [](const C3Vector &v, M2GpuTrackPack &p) {
                p.valuesVec4.insert(p.valuesVec4.end(), {v.x, v.y, v.z, 0.0f});
            });
        st.alphaTrackSlice = packM2PartTrack<fixed16>(old.alphaTrack, pack, TrackValueKind::Float,
            [](const fixed16 &v, M2GpuTrackPack &p) {
                p.valuesFloat.push_back(convertHelper<fixed16, float>(v));
            });
        st.scaleTrackSlice = packM2PartTrack<C2Vector>(old.scaleTrack, pack, TrackValueKind::Vec4,
            [](const C2Vector &v, M2GpuTrackPack &p) {
                p.valuesVec4.insert(p.valuesVec4.end(), {v.x, v.y, 0.0f, 0.0f});
            });
        st.headCellTrackSlice = packM2PartTrack<uint16_t>(old.headCellTrack, pack, TrackValueKind::Float,
            [](const uint16_t &v, M2GpuTrackPack &p) { p.valuesFloat.push_back((float)v); });
        st.tailCellTrackSlice = packM2PartTrack<uint16_t>(old.tailCellTrack, pack, TrackValueKind::Float,
            [](const uint16_t &v, M2GpuTrackPack &p) { p.valuesFloat.push_back((float)v); });
        if (exp2Rec != nullptr) {
            st.alphaCutoffSlice = packM2PartTrack<fixed16>(exp2Rec->alphaCutoff, pack, TrackValueKind::Float,
                [](const fixed16 &v, M2GpuTrackPack &p) {
                    p.valuesFloat.push_back(convertHelper<fixed16, float>(v));
                });
        } else {
            st.alphaCutoffSlice = {};
        }

        pack.particleStatics.push_back(st);

        // Spline (3) and bone (4) generators stay on the CPU path in v1
        pack.particleGpuSupported.push_back((old.emitterType == 1 || old.emitterType == 2) ? 1 : 0);

        // Capacity estimate: emission-rate max * lifespan max, generous headroom,
        // hard-capped at the same limit the CPU path uses.
        float maxRate = maxTrackValue(old.emissionRate) + old.emissionRateVary;
        float maxLife = maxTrackValue(old.lifespan) + old.lifespanVary;
        int32_t capacity = (int32_t)ceilf(maxRate * maxLife * 2.0f) + 16;
        capacity = std::max(capacity, 16);
        capacity = std::min(capacity, GPU_MAX_PARTICLES_PER_EMITTER);
        pack.particleCapacity.push_back(capacity);
    }

    return packValid;
}

// Ribbons: packs M2Ribbon records + the constants CRibbonEmitter::Initialize derives.
bool packRibbonEmitters(M2GpuTrackPack &pack,
                        const M2Data *m2Data,
                        const M2Array<M2Sequence> *sequences) {
    bool packValid = true;

    auto &ribbons = m2Data->ribbon_emitters;
    for (int32_t i = 0; i < ribbons.size; i++) {
        M2Ribbon *rb = ribbons.getElement(i);

        GpuM2RibbonStatic st;
        std::memset(&st, 0, sizeof(st));

        st.boneIndex = (int32_t)rb->boneIndex;
        st.position[0] = rb->position.x;
        st.position[1] = rb->position.y;
        st.position[2] = rb->position.z;

        // CRibbonEmitter::Initialize(edgesPerSecond, edgeLifetime, white, unitRect, cols, rows)
        float edgesPerSec = ceilf(rb->edgesPerSecond);
        float edgeLifeSpan = fmaxf(0.25f, rb->edgeLifetime);
        st.edgesPerSec = edgesPerSec;
        st.edgeLifeSpan = edgeLifeSpan;
        st.ooLifeSpan = 1.0f / edgeLifeSpan;
        st.gravity = rb->gravity;
        st.textureRows = rb->textureRows;
        st.textureCols = rb->textureCols;
        st.priorityPlane = rb->priorityPlane;

        // unitRect (0,0,1,1) as in initRibbonEmitters
        st.texBoxMinX = 0.0f; st.texBoxMinY = 0.0f; st.texBoxMaxX = 1.0f; st.texBoxMaxY = 1.0f;
        st.tmpDU = (st.texBoxMaxX - st.texBoxMinX) / (float)std::max<int>(st.textureCols, 1);
        st.tmpDV = (st.texBoxMaxY - st.texBoxMinY) / (float)std::max<int>(st.textureRows, 1);

        auto packVec3Track = [&](const M2Track<C3Vector> &track) -> int32_t {
            return packM2Track<C3Vector>(track, pack, sequences, TrackValueKind::Vec4,
                [](const C3Vector &v, M2GpuTrackPack &p) {
                    p.valuesVec4.insert(p.valuesVec4.end(), {v.x, v.y, v.z, 0.0f});
                });
        };
        auto packFixed16Track = [&](const M2Track<fixed16> &track) -> int32_t {
            return packM2Track<fixed16>(track, pack, sequences, TrackValueKind::Float,
                [](const fixed16 &v, M2GpuTrackPack &p) {
                    p.valuesFloat.push_back(convertHelper<fixed16, float>(v));
                });
        };
        auto packFloatTrack = [&](const M2Track<float> &track) -> int32_t {
            return packM2Track<float>(track, pack, sequences, TrackValueKind::Float,
                [](const float &v, M2GpuTrackPack &p) { p.valuesFloat.push_back(v); });
        };
        auto packUint16Track = [&](const M2Track<uint16_t> &track) -> int32_t {
            return packM2Track<uint16_t>(track, pack, sequences, TrackValueKind::Float,
                [](const uint16_t &v, M2GpuTrackPack &p) { p.valuesFloat.push_back((float)v); });
        };
        auto packUcharTrack = [&](const M2Track<unsigned char> &track) -> int32_t {
            return packM2Track<unsigned char>(track, pack, sequences, TrackValueKind::Float,
                [](const unsigned char &v, M2GpuTrackPack &p) { p.valuesFloat.push_back((float)v); });
        };

        st.colorTrack = packVec3Track(rb->colorTrack);
        st.alphaTrack = packFixed16Track(rb->alphaTrack);
        st.heightAboveTrack = packFloatTrack(rb->heightAboveTrack);
        st.heightBelowTrack = packFloatTrack(rb->heightBelowTrack);
        st.texSlotTrack = packUint16Track(rb->texSlotTrack);
        st.visibilityTrack = packUcharTrack(rb->visibilityTrack);

        pack.ribbonStatics.push_back(st);
    }

    return packValid;
}

} // namespace

M2GpuTrackPack buildM2GpuTrackPack(CBoneMasterData &boneMasterData,
                                   const M2Data *m2Data,
                                   const EXP2 *exp2) {
    M2GpuTrackPack pack;

    auto *skelData = boneMasterData.getSkelData();
    auto *parentSkelData = boneMasterData.getParentSkelData();

    if (skelData == nullptr || skelData->m_sequences == nullptr || skelData->m_sequences->size <= 0) {
        // No sequences: nothing animates; the GPU path would never be dispatched anyway.
        pack.valid = false;
        return pack;
    }

    bool valid = packBank(skelData, pack, false, pack.ownBoneCount);
    if (valid && parentSkelData != nullptr) {
        valid = packBank(parentSkelData, pack, true, pack.parentBoneCount);
        // Parent bank is indexed by the same bone indices as the own bank when
        // animations come from the parent skeleton (see AnimationManager::calcBoneMatrix).
        if (valid && pack.parentBoneCount > 0 && pack.parentBoneCount < pack.ownBoneCount) {
            valid = false;
        }
    }

    if (valid && m2Data != nullptr) {
        valid = packParticleEmitters(pack, m2Data, exp2, skelData->m_sequences);
        if (valid) {
            valid = packRibbonEmitters(pack, m2Data, skelData->m_sequences);
        }
    }

    pack.valid = valid;
    return pack;
}

int32_t countGpuLoadedSequences(CBoneMasterData &boneMasterData) {
    int32_t count = 0;
    for (auto *skelData : {boneMasterData.getSkelData(), boneMasterData.getParentSkelData()}) {
        if (skelData == nullptr || skelData->m_sequences == nullptr) continue;
        for (int32_t i = 0; i < skelData->m_sequences->size; i++) {
            if ((skelData->m_sequences->getElement(i)->flags & 0x20) != 0) count++;
        }
    }
    return count;
}
