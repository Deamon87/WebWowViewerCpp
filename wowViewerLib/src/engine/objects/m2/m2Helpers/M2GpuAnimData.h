//
// GPU M2 animation: shared data layouts (C++/slang mirror) and the load-time track packer.
//
// The layouts below are consumed by shaders/slang/common/m2TrackEval.slang and
// shaders/slang/compute/m2Animation.comp.slang. All structs are std430-friendly
// (4-byte scalars, explicit vec4 storage).
//

#ifndef AWEBWOWVIEWERCPP_M2GPUANIMDATA_H
#define AWEBWOWVIEWERCPP_M2GPUANIMDATA_H

#include <cstdint>
#include <vector>
#include <memory>

#include "../../../persistance/header/M2FileHeader.h" // M2Data, EXP2

class CBoneMasterData;

// Max global sequences per bank supported by the GPU path. Objects with more
// global sequences fall back to the CPU path.
constexpr int GPU_ANIM_MAX_GLOBAL_SEQUENCES = 32;
// Max bone-chain depth supported by the GPU bone compute (parent links are
// replayed root->leaf per thread). Objects with deeper chains fall back to CPU.
constexpr int GPU_ANIM_MAX_BONE_CHAIN_DEPTH = 64;
// Mirrors MAX_PARTICLES_PER_EMITTER (IMapSceneBufferCreate.h) — hard cap per emitter.
constexpr int32_t GPU_MAX_PARTICLES_PER_EMITTER = 2000;

// Static per-bone data (both banks concatenated: own bones first, then parent-skel bones)
struct GpuM2BoneStatic {
    float pivot[3];
    int32_t parentBone;
    uint32_t flagsRaw;
    int32_t transTrack; // index into trackHeaders; -1 = track absent (identity factor)
    int32_t rotTrack;   // values are quats packed as vec4(x,y,z,w), converted+normalized at pack time
    int32_t scaleTrack;
};
static_assert(sizeof(GpuM2BoneStatic) % 16 == 0, "std430 alignment");

struct GpuM2TrackHeader {
    int32_t interpolationType;
    int32_t globalSequence;   // -1 = none
    int32_t seqSliceOffset;   // first GpuM2SeqSlice
    int32_t seqSliceCount;    // == sequence count of the owning bank (may exceed actual slices)
};
static_assert(sizeof(GpuM2TrackHeader) == 16, "std430 alignment");

struct GpuM2SeqSlice {
    uint32_t timeOffset;   // into timestamps[]
    uint32_t valueOffset;  // into valuesVec4[] or valuesFloat[] (depends on the consuming track)
    uint32_t elemCount;    // 0 = use default value
    uint32_t pad;
};
static_assert(sizeof(GpuM2SeqSlice) == 16, "std430 alignment");

// Per-age track (M2PartTrack) slice: timestamps are converted to float seconds at
// pack time into the partTimes pool; values go to the typed value pools.
// Uses the same GpuM2SeqSlice layout (timeOffset indexes partTimes here).

// Static per-particle-emitter data (M2ParticleOld scalars + track references).
// All values are pre-decoded/converted so shaders never touch packed file formats.
struct GpuM2ParticleStatic {
    uint32_t flags;
    int32_t bone;
    float position[3];
    int32_t emitterType;
    int32_t blendingType;
    int32_t particleColorIndex;

    float lifespanVary;
    float emissionRateVary;
    float scaleVary[2];

    float tailLength;
    float twinkleSpeed;
    float twinklePercent;
    float twinkleScaleMin;
    float twinkleScaleMax;
    float burstMultiplier;
    float drag;
    float baseSpin;
    float baseSpinVary;
    float spin;
    float spinVary;

    float windVector[3];
    float windTime;

    float followSpeed1;
    float followScale1;
    float followSpeed2;
    float followScale2;
    float followMult;   // precomputed (followScale2-followScale1)/(followSpeed2-followSpeed1) or 0
    float followBase;

    int32_t textureColMask;
    int32_t textureColBits;
    int32_t textureIndexMask;
    int32_t particleType;
    float texScaleX;
    float texScaleY;

    float multiTextureParam0[4]; // 2 × vec2 (decoded from vector_2fp_6_9)
    float multiTextureParam1[4];
    float multiTextureParamX0;   // paramXTransform(multiTextureParamX[0])
    float multiTextureParamX1;

    float zSourceExp2;      // EXP2 static zSource (valid when hasExp2 != 0)
    int32_t hasExp2;

    // Per-frame track headers (indices into the trackHeaders pool; -1 = absent).
    // All float-pool tracks except gravity (vec4 pool when gravityIsCompressed).
    int32_t emissionSpeedTrack;
    int32_t speedVariationTrack;
    int32_t verticalRangeTrack;
    int32_t horizontalRangeTrack;
    int32_t gravityTrack;
    int32_t gravityIsCompressed;
    int32_t lifespanTrack;
    int32_t emissionRateTrack;
    int32_t emissionAreaXTrack; // emissionAreaLength
    int32_t emissionAreaYTrack; // emissionAreaWidth
    int32_t zSourceTrack;       // absent when hasExp2
    int32_t enabledInTrack;

    // Per-age tracks (M2PartTrack); slices into partTimes + value pools
    GpuM2SeqSlice colorTrackSlice;    // vec4 pool (rgb)
    GpuM2SeqSlice alphaTrackSlice;    // float pool
    GpuM2SeqSlice scaleTrackSlice;    // vec4 pool (xy)
    GpuM2SeqSlice headCellTrackSlice; // float pool (cell index as float, integer-lerped)
    GpuM2SeqSlice tailCellTrackSlice; // float pool
    GpuM2SeqSlice alphaCutoffSlice;   // float pool (EXP2); elemCount==0 when absent

    int32_t pad[5];
};
static_assert(sizeof(GpuM2ParticleStatic) % 16 == 0, "std430 alignment");

// Per-emitter per-frame animated properties, written by the m2Animation compute pass
// (mirrors CGeneratorAniProp + isEnabled).
struct GpuM2ParticleFrameProps {
    float emissionSpeed;
    float speedVariation;
    float verticalRange;
    float horizontalRange;
    float gravity[3];
    float lifespan;
    float emissionRate;
    float emissionAreaX;
    float emissionAreaY;
    float zSource;
    int32_t isEnabled;
    int32_t pad[3];
};
static_assert(sizeof(GpuM2ParticleFrameProps) % 16 == 0, "std430 alignment");

// Persistent per-emitter simulation state (GPU-written, survives frames).
struct GpuParticleState {
    float emitterModelMatrix[16];
    float s_particleToView[16];
    float p_quadToView0[4];      // mat3 columns as vec4
    float p_quadToView1[4];
    float p_quadToView2[4];
    float p_quadToViewZVector[4];

    float prevPosition[3];
    float burstTime;
    float burstVec[3];
    float inheritedScale;
    float fullPosDelta[3];
    int32_t curBuf;              // 0 = particlesA, 1 = particlesB
    float deltaPosition[3];
    int32_t particleCount;       // alive count in the current buffer

    float emission;              // emission accumulator
    uint32_t emitterSeedValue;   // CRndSeed of the emitter (m_seed)
    uint32_t emitterSeedAccum;
    uint32_t generatorSeedValue; // CRndSeed of the generator
    uint32_t generatorSeedAccum;

    // Pool indices (static, set at allocation)
    int32_t staticsIndex;        // GpuM2ParticleStatic slot
    int32_t propsIndex;          // GpuM2ParticleFrameProps slot
    int32_t particleOffset;      // first GpuParticle element of this emitter in particlesA (pool-absolute)
    int32_t capacity;
    int32_t placementMatIndex;   // index into the renderer's placementMat SSBO
    int32_t boneOutOffset;       // owning object's region base in the bone-matrix SSBO
    // CPU-emitter init-time random cell offset (ParticleEmitter::m_randomizedTextureIndexMask)
    int32_t randomizedTextureIndexMask;
    int32_t particleOffsetB;     // same emitter's first GpuParticle element in particlesB (pool-absolute; A/B chunk offsets may differ)
    int32_t pad[3];
};
static_assert(sizeof(GpuParticleState) % 16 == 0, "std430 alignment");

// One simulated particle (mirrors CParticle2)
struct GpuParticle {
    float position[3];
    float age;
    float velocity[3];
    int32_t state;
    float texPos[4];   // [0].xy, [1].xy
    float texVel[4];
    uint32_t seed;
    int32_t isDead;
    int32_t pad[2];
};
static_assert(sizeof(GpuParticle) % 16 == 0, "std430 alignment");

// Per-emitter seed snapshot captured from the CPU emitter at GPU-init time, so the
// GPU RNG streams continue the exact sequences the CPU emitter would have produced.
struct M2GpuEmitterSeeds {
    uint32_t emitterSeedValue = 0;
    uint32_t emitterSeedAccum = 0;
    uint32_t generatorSeedValue = 0;
    uint32_t generatorSeedAccum = 0;
};

// ---- Ribbons ----

// Static per-ribbon data (M2Ribbon scalars + track references).
struct GpuM2RibbonStatic {
    float position[3];
    int32_t boneIndex;
    float edgesPerSec;        // ceil(edgesPerSecond)
    float edgeLifeSpan;       // max(0.25, edgeLifetime)
    float gravity;
    int32_t textureRows;
    int32_t textureCols;
    float texBoxMinX, texBoxMinY, texBoxMaxX, texBoxMaxY;  // unitRect
    float tmpDU, tmpDV;       // cell sizes
    float ooLifeSpan;         // 1/edgeLifeSpan
    int32_t priorityPlane;
    // tracks (trackHeaders pool indices; -1 = absent)
    int32_t colorTrack;       // vec4 pool
    int32_t alphaTrack;       // float pool
    int32_t heightAboveTrack; // float pool
    int32_t heightBelowTrack; // float pool
    int32_t texSlotTrack;     // float pool (uint16 values, integer-lerped)
    int32_t visibilityTrack;  // float pool (uchar values)
    int32_t pad[1];
};
static_assert(sizeof(GpuM2RibbonStatic) % 16 == 0, "std430 alignment");

// Per-ribbon per-frame animated properties, written by the m2Animation compute pass
// (mirrors CRibbonEmitter's SetColor/SetAlpha/SetAbove/SetBelow/SetTexSlot/SetDataEnabled).
struct GpuM2RibbonFrameProps {
    float color[3];           // evaluated colorTrack (linear 0..1, pre-255 scaling)
    float alpha;              // evaluated alphaTrack
    float above;
    float below;
    float texSlotBox[4];      // minx, miny, maxx, maxy (SetTexSlot result)
    int32_t dataEnabled;
    int32_t pad[5];
};
static_assert(sizeof(GpuM2RibbonFrameProps) % 16 == 0, "std430 alignment");

// Per-edge live content, written by the ribbon sim compute pass.
// Dead/out-of-arc edges are never referenced by the (rewritten-per-frame) index buffer.
struct GpuRibbonEdge {
    float belowPos[3];
    float age;                // seconds since the edge was written
    float abovePos[3];
    float pad;
};
static_assert(sizeof(GpuRibbonEdge) % 16 == 0, "std430 alignment");

// Persistent per-ribbon simulation state (GPU-written, survives frames).
// All the vec3 groups are float[4] for alignment simplicity.
struct GpuRibbonState {
    int32_t readPos;
    int32_t writePos;
    float startTime;
    int32_t posSet;
    int32_t singletonUpdated;
    int32_t pad0[3];

    float prevPos[4];
    float currPos[4];
    float prevVertical[4];
    float currVertical[4];
    float prevDir[4];
    float currDir[4];
    float prevDirScaled[4];
    float currDirScaled[4];
    float below0[4];
    float below1[4];
    float above0[4];
    float above1[4];

    // Pool indices (static, set at allocation)
    int32_t staticsIndex;     // GpuM2RibbonStatic slot
    int32_t propsIndex;       // GpuM2RibbonFrameProps slot
    int32_t edgesOffset;      // first GpuRibbonEdge in the edges pool
    int32_t edgeCount;
    int32_t placementMatIndex;
    int32_t boneOutOffset;
    int32_t iboOffset;        // first uint16 in the ribbon index pool
    int32_t iboCount;         // 4 * edgeCount
};
static_assert(sizeof(GpuRibbonState) % 16 == 0, "std430 alignment");

// Per-object animation state: static chunk offsets (written at allocation) +
// dynamic sequencing state (rewritten every frame from AnimationManager).
struct GpuM2AnimState {
    float modelViewMat[16];
    float invModelViewMat[16];

    int32_t animIndex;
    float   animTime;              // ms
    int32_t animFoundInParent;
    int32_t nextAnimIndex;         // -1 = no blending
    float   nextAnimTime;          // ms
    int32_t nextAnimFoundInParent;
    float   blendFactor;

    // Static chunk offsets (element indices into the renderer's SSBO pools)
    int32_t boneStaticOffset;      // first GpuM2BoneStatic (own bank); parent bank follows
    int32_t ownBoneCount;
    int32_t parentBoneCount;
    int32_t boneOutOffset;         // mat4 element index into the bone-matrix SSBO
    int32_t trackHeaderOffset;
    int32_t seqSliceOffset;
    int32_t timestampOffset;
    int32_t valuesVec4Offset;
    int32_t valuesFloatOffset;
    int32_t seqDurationOffset;     // uints; own bank first, parent bank follows
    int32_t ownSeqCount;
    int32_t parentSeqCount;
    int32_t globalLoopOffset;      // uints; own bank first, parent bank follows
    int32_t ownGsCount;
    int32_t parentGsCount;

    // Particle emitter regions (per-object, in the renderer's particle pools)
    int32_t particleStaticsOffset; // first GpuM2ParticleStatic
    int32_t particlePropsOffset;   // first GpuM2ParticleFrameProps (written by m2Animation)
    int32_t particleEmitterCount;
    int32_t partTimesOffset;       // first uint32 in the partTimes pool

    // Ribbon regions (per-object, in the renderer's ribbon pools)
    int32_t ribbonStaticsOffset;   // first GpuM2RibbonStatic
    int32_t ribbonPropsOffset;     // first GpuM2RibbonFrameProps (written by m2Animation)
    int32_t ribbonCount;
    int32_t pad3[3];

    float   gsTimes[GPU_ANIM_MAX_GLOBAL_SEQUENCES];       // own bank global sequence times
    float   parentGsTimes[GPU_ANIM_MAX_GLOBAL_SEQUENCES]; // parent bank global sequence times
};
static_assert(sizeof(GpuM2AnimState) % 16 == 0, "std430 alignment");

// CPU-side packed track data for one model, ready to be uploaded into the
// renderer's GPU pools. Pure data — no GPU API dependency.
struct M2GpuTrackPack {
    std::vector<GpuM2BoneStatic> boneStatics;    // own bank first, then parent bank
    int32_t ownBoneCount = 0;
    int32_t parentBoneCount = 0;

    std::vector<GpuM2TrackHeader> trackHeaders;
    std::vector<GpuM2SeqSlice> seqSlices;
    std::vector<uint32_t> timestamps;
    std::vector<float> valuesVec4;               // flat; 4 floats per logical element
    std::vector<float> valuesFloat;

    std::vector<uint32_t> seqDurations;          // own bank first, then parent bank
    int32_t ownSeqCount = 0;
    int32_t parentSeqCount = 0;

    std::vector<uint32_t> globalLoops;           // own bank first, then parent bank
    int32_t ownGsCount = 0;
    int32_t parentGsCount = 0;

    // Particle emitters (per model). gpuSupported=false emitters (spline/bone
    // generators) stay on the CPU path even when the object animates on GPU.
    std::vector<GpuM2ParticleStatic> particleStatics;
    std::vector<uint8_t> particleGpuSupported;
    std::vector<int32_t> particleCapacity;       // per emitter: ring capacity estimate
    std::vector<uint32_t> partTimes;             // raw fixed16 M2PartTrack timestamps

    // Ribbons (per model; all go through the GPU path when the object is GPU-animated)
    std::vector<GpuM2RibbonStatic> ribbonStatics;

    // False when the model cannot be fully packed (bone chain deeper than
    // GPU_ANIM_MAX_BONE_CHAIN_DEPTH, too many global sequences, no sequences at
    // all) — such objects stay on the CPU animation path.
    // Lazy (.anim) sequences do NOT invalidate the pack: their slices are packed
    // empty and re-packed when they stream in (see IM2GpuTrackSet::syncTrackData).
    bool valid = false;

    // Number of resident sequences (flag 0x20) per bank at pack time. M2Object
    // re-runs the pack when this count grows (lazy .anim streaming).
    int32_t loadedSeqCountOwn = 0;
    int32_t loadedSeqCountParent = 0;

    size_t trackDataBytes() const {
        return boneStatics.size() * sizeof(GpuM2BoneStatic)
             + trackHeaders.size() * sizeof(GpuM2TrackHeader)
             + seqSlices.size() * sizeof(GpuM2SeqSlice)
             + timestamps.size() * sizeof(uint32_t)
             + valuesVec4.size() * sizeof(float)
             + valuesFloat.size() * sizeof(float)
             + seqDurations.size() * sizeof(uint32_t)
             + globalLoops.size() * sizeof(uint32_t)
             + particleStatics.size() * sizeof(GpuM2ParticleStatic)
             + partTimes.size() * sizeof(uint32_t);
    }
};

// Packs all bone + particle-emitter animation data of a model into GPU-ready arrays.
// Pure CPU; performs the same value conversions as convertHelper<> in animate.h
// (Quat16 -> normalized quat vec4(x,y,z,w)) so shaders only lerp/slerp floats.
// m2Data/exp2 supply the particle emitter records (may be null -> no emitters).
// Lazy (.anim) sequences are packed as empty slices; re-run the build when more
// sequences become resident — the renderer's shared track set re-uploads from the
// fresh pack (IM2GpuTrackSet::syncTrackData, once per model, not per instance).
M2GpuTrackPack buildM2GpuTrackPack(CBoneMasterData &boneMasterData,
                                   const M2Data *m2Data,
                                   const EXP2 *exp2);

// Counts sequences with resident data (flag 0x20) across both skeleton banks.
// Used to detect .anim streaming completion between frames.
int32_t countGpuLoadedSequences(CBoneMasterData &boneMasterData);

#endif //AWEBWOWVIEWERCPP_M2GPUANIMDATA_H
