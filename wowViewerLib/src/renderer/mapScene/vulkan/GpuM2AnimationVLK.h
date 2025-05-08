//
// Created by Deamon on 2026-08-18.
//
// GPU M2 animation + particle-simulation passes (bindless Vulkan renderer).
//
// Owns the SSBO pools with packed static track data (built per model by
// buildM2GpuTrackPack), the per-object GpuM2AnimState slots, the particle emitter
// pools (statics/props/states/particles) and the compute passes:
//   - m2Animation: bone matrices (into the renderer's bindless bone SSBO) +
//     per-frame particle emitter property tracks;
//   - particleSimulate: full particle sim (age/emit/integrate) into ping-pong
//     particle pools; the pull-model m2ParticleGpuShader vertex shader expands
//     quads directly from those pools.
//
// Buffer growth: all pools are GBufferVLK with offset-allocator sub-allocation;
// when a pool runs out, getSubBuffer() grows it (GBufferVLK::resize) and the
// descriptor set is re-bound automatically through the DSBindable on-change path.
//

#ifndef AWEBWOWVIEWERCPP_GPUM2ANIMATIONVLK_H
#define AWEBWOWVIEWERCPP_GPUM2ANIMATIONVLK_H

#include <memory>
#include <vector>
#include <cstdint>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <mathfu/glsl_mappings.h>
#include "../../../gapi/vulkan/GDeviceVulkan.h"
#include "../../../gapi/vulkan/buffers/GBufferVLK.h"
#include "../../../gapi/vulkan/buffers/CBufferChunkVLK.h"
#include "../../../gapi/vulkan/materials/ComputeMaterialBuilderVLK.h"
#include "../../../engine/objects/m2/m2Helpers/M2GpuAnimData.h"
#include "../materials/IMaterialStructs.h"

class CmdBufRecorder;
class Config;

// Model-shared static track data: one per source model, referenced by the
// per-object M2GpuAnimDataVLK instances. Owns the static pool chunks and their
// offsets; handles .anim streaming re-uploads (syncTrackData) exactly once per
// model regardless of how many instances detect the newly resident sequences.
class M2GpuTrackSetVLK : public IM2GpuTrackSet {
public:
    // The model-static chunk offsets, mirrored into every referencing object's
    // GpuM2AnimState by M2GpuAnimDataVLK::getStateForWrite().
    struct SharedOffsets {
        int32_t boneStaticOffset = 0;
        int32_t ownBoneCount = 0;
        int32_t parentBoneCount = 0;
        int32_t trackHeaderOffset = 0;
        int32_t seqSliceOffset = 0;
        int32_t timestampOffset = 0;
        int32_t valuesVec4Offset = 0;
        int32_t valuesFloatOffset = 0;
        int32_t seqDurationOffset = 0;
        int32_t ownSeqCount = 0;
        int32_t parentSeqCount = 0;
        int32_t globalLoopOffset = 0;
        int32_t ownGsCount = 0;
        int32_t parentGsCount = 0;
        int32_t particleStaticsOffset = 0;
        int32_t partTimesOffset = 0;
        int32_t ribbonStaticsOffset = 0;
    };

    // Immutable model info derived from the creation-time pack. Per-instance
    // allocation reads this without locking (counts/flags never change on
    // .anim streaming — only track values do).
    struct ModelInfo {
        int32_t ownBoneCount = 0;
        int32_t particleEmitterCount = 0;
        int32_t ribbonCount = 0;
        bool needsColorRepl = false;
        std::vector<uint8_t> particleGpuSupported;
        std::vector<int32_t> particleCapacity;
        std::vector<uint8_t> particleQuadsPerParticle;
        std::vector<float> ribbonEdgesPerSec;
        std::vector<float> ribbonEdgeLifeSpan;
    };
    ModelInfo m_modelInfo;

    int32_t getLoadedSeqCount() const override { return m_loadedSeqCount.load(std::memory_order_acquire); }
    uint32_t getDataGeneration() const override { return m_dataGeneration.load(std::memory_order_acquire); }

    void syncTrackData(int32_t loadedSeqCount,
                       const std::function<M2GpuTrackPack()> &packBuilder) override;

    // Locked snapshot of the shared offsets (they move on .anim streaming).
    SharedOffsets getSharedOffsets() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_offsets;
    }

    // Guards m_offsets and the chunk replacement in syncTrackData.
    mutable std::mutex m_mutex;
    SharedOffsets m_offsets;
    std::atomic<int32_t> m_loadedSeqCount{0};
    std::atomic<uint32_t> m_dataGeneration{0};

    // Keeps the static-data sub-buffer allocations alive for the track set's
    // lifetime (GSubBufferVLK destructor deallocates the range from the pool).
    std::vector<std::shared_ptr<IBufferVLK>> m_keepAlive;

    // Replaceable track-data pools/chunks for .anim streaming (syncTrackData).
    // Held as HGBufferVLK so the pools stay alive regardless of renderer teardown
    // order; replaced chunks free their pool range through the deferred
    // deallocation queue (safe for in-flight frames).
    HGBufferVLK m_seqSlicesPool;
    HGBufferVLK m_timestampsPool;
    HGBufferVLK m_valuesVec4Pool;
    HGBufferVLK m_valuesFloatPool;
    HGBufferVLK m_particleStaticsPool;
    HGBufferVLK m_ribbonStaticsPool;
    std::shared_ptr<IBufferVLK> m_chunkSeqSlices;
    std::shared_ptr<IBufferVLK> m_chunkTimestamps;
    std::shared_ptr<IBufferVLK> m_chunkValuesVec4;
    std::shared_ptr<IBufferVLK> m_chunkValuesFloat;
};

class M2GpuAnimDataVLK : public IM2GpuAnimData {
public:
    // Prefills the static (chunk offset) fields from the shared track set and
    // overlays the per-object fields; the caller fills the dynamic ones.
    GpuM2AnimState &getStateForWrite() override {
        GpuM2AnimState &state = m_stateChunk->getObject();
        state = {};
        auto shared = m_trackSet->getSharedOffsets();
        state.boneStaticOffset = shared.boneStaticOffset;
        state.ownBoneCount = shared.ownBoneCount;
        state.parentBoneCount = shared.parentBoneCount;
        state.trackHeaderOffset = shared.trackHeaderOffset;
        state.seqSliceOffset = shared.seqSliceOffset;
        state.timestampOffset = shared.timestampOffset;
        state.valuesVec4Offset = shared.valuesVec4Offset;
        state.valuesFloatOffset = shared.valuesFloatOffset;
        state.seqDurationOffset = shared.seqDurationOffset;
        state.ownSeqCount = shared.ownSeqCount;
        state.parentSeqCount = shared.parentSeqCount;
        state.globalLoopOffset = shared.globalLoopOffset;
        state.ownGsCount = shared.ownGsCount;
        state.parentGsCount = shared.parentGsCount;
        state.particleStaticsOffset = shared.particleStaticsOffset;
        state.partTimesOffset = shared.partTimesOffset;
        state.ribbonStaticsOffset = shared.ribbonStaticsOffset;

        state.boneOutOffset = m_boneOutOffset;
        state.particlePropsOffset = m_particlePropsOffset;
        state.particleEmitterCount = m_trackSet->m_modelInfo.particleEmitterCount;
        state.ribbonPropsOffset = m_ribbonPropsOffset;
        state.ribbonCount = m_trackSet->m_modelInfo.ribbonCount;

        // Defaults until fillGpuAnimState writes the real sequencing state
        state.animIndex = 0;
        state.nextAnimIndex = -1;
        state.blendFactor = 1.0f;
        return state;
    }

    int32_t getStateIndex() const override {
        return (int32_t)(m_stateChunk->getIndex());
    }

    std::shared_ptr<IM2GpuTrackSet> getTrackSet() override { return m_trackSet; }

    bool getParticleEmitterBindInfo(int emitterLocalIndex, GpuParticleEmitterBindInfo &out) const override {
        if (emitterLocalIndex < 0 || emitterLocalIndex >= (int)m_emitterInfos.size()) return false;
        const auto &info = m_emitterInfos[emitterLocalIndex];
        if (info.stateIndex < 0) return false;

        auto shared = m_trackSet->getSharedOffsets();
        out.stateIndex = info.stateIndex;
        out.particleOffset = info.particleOffset;
        out.capacity = info.capacity;
        out.staticsIndex = shared.particleStaticsOffset + emitterLocalIndex;
        out.propsIndex = m_particlePropsOffset + emitterLocalIndex;
        out.colorReplOffset = m_colorReplValid ? m_colorReplOffset : -1;
        out.quadsPerParticle = info.quadsPerParticle;
        out.valuesVec4Offset = shared.valuesVec4Offset;
        out.valuesFloatOffset = shared.valuesFloatOffset;
        out.partTimesOffset = shared.partTimesOffset;
        return true;
    }

    bool getRibbonBindInfo(int ribbonLocalIndex, GpuRibbonEmitterBindInfo &out) const override {
        if (ribbonLocalIndex < 0 || ribbonLocalIndex >= (int)m_ribbonInfos.size()) return false;
        const auto &info = m_ribbonInfos[ribbonLocalIndex];

        out.stateIndex = info.stateIndex;
        out.propsIndex = m_ribbonPropsOffset + ribbonLocalIndex;
        out.edgesOffset = info.edgesOffset;
        out.edgeCount = info.edgeCount;
        return true;
    }

    void updateParticleColorReplacements(const std::array<std::array<mathfu::vec4, 3>, 3> &colors, bool valid) override {
        if (m_colorReplChunk == nullptr) return;
        mathfu::vec4 *data = static_cast<mathfu::vec4*>(m_colorReplChunk->getPointer());
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                data[i * 3 + j] = valid ? colors[i][j] : mathfu::vec4(0, 0, 0, 0);
            }
        }
        m_colorReplValid = valid;
    }

    std::shared_ptr<M2GpuTrackSetVLK> m_trackSet;
    std::shared_ptr<CBufferChunkVLK<GpuM2AnimState>> m_stateChunk;

    // Per-object pool offsets (the shared/static ones live in m_trackSet)
    int32_t m_boneOutOffset = 0;
    int32_t m_particlePropsOffset = 0;
    int32_t m_ribbonPropsOffset = 0;

    struct EmitterGpuInfo {
        int32_t stateIndex = -1;       // slot in the particle-states pool (-1 = CPU path)
        int32_t particleOffset = 0;
        int32_t capacity = 0;
        int32_t quadsPerParticle = 1;
    };
    std::vector<EmitterGpuInfo> m_emitterInfos;

    struct RibbonGpuInfo {
        int32_t stateIndex = -1;       // slot in the ribbon-states pool
        int32_t edgesOffset = 0;
        int32_t edgeCount = 0;
    };
    std::vector<RibbonGpuInfo> m_ribbonInfos;

    // Per-object particle-color-replacement chunk (9 vec4) in the colorRepl pool
    std::shared_ptr<IBufferVLK> m_colorReplChunk = nullptr;
    int32_t m_colorReplOffset = -1;
    bool m_colorReplValid = false;

    // Keeps the per-object sub-buffer allocations alive for the object's lifetime
    // (GSubBufferVLK destructor deallocates the range from the pool).
    std::vector<std::shared_ptr<IBufferVLK>> m_keepAlive;
};

class GpuM2AnimationVLK {
public:
    GpuM2AnimationVLK(const HGDeviceVLK &device, Config *config);

    // boneMatrixBuffer / placementMatBuffer are the renderer's bindless SSBOs
    // (bone SSBO is written by the compute pass and read by the M2 vertex shaders).
    void setup(std::vector<std::reference_wrapper<HGBufferVLK>> &allBuffers,
               const HGBufferVLK &boneMatrixBuffer,
               const HGBufferVLK &placementMatBuffer);

    // Allocates the per-object state slot and sim data; uploads the packed static
    // track data on first use of a model. modelKey identifies the source geometry
    // (M2Geom pointer); packBuilder is invoked at most once per model — only when
    // no shared track set exists for the key yet. Returns nullptr when the model
    // cannot be packed for the GPU path (negative-cached per key).
    // boneOutOffset = element index of the object's region in the bone-matrix SSBO
    // (its m_bonesData chunk index); placementMatIndex likewise for the placement SSBO.
    // ribbonGpuIndexBuffers: per-ribbon index-buffer chunks created at load time via
    // createM2RibbonGpuIndexBuffer (the ribbon sim rewrites their content per frame).
    // Thread-safe: M2Object::update runs under tbb::parallel_for.
    std::shared_ptr<IM2GpuAnimData> createAnimData(
        const void *modelKey,
        const std::function<M2GpuTrackPack()> &packBuilder,
        int32_t boneOutOffset,
        int32_t placementMatIndex,
        const std::vector<M2GpuEmitterSeeds> &emitterSeeds,
        const std::vector<int32_t> &emitterRandomizedTextureIndexMasks,
        const std::vector<std::shared_ptr<IBuffer>> &ribbonGpuIndexBuffers);

    // Per-ribbon GPU index buffer chunk in the ribbon index pool (the ribbon sim
    // rewrites its content per frame). The chunk doubles as an HGIndexBuffer.
    std::shared_ptr<IBuffer> createRibbonGpuIndexBuffer(int32_t edgeCount);

    // Update stage: queues the lists of anim-state / particle-state / ribbon-state
    // indices to dispatch this frame.
    void updateDispatchList(const std::vector<uint32_t> &stateIndices);
    void updateEmitterDispatchList(const std::vector<uint32_t> &emitterStateIndices);
    void updateRibbonDispatchList(const std::vector<uint32_t> &ribbonStateIndices);

    // Draw stage: records the compute dispatches + barriers before M2 draws.
    // Counts are captured at update time (the draw stage may be recorded while
    // the next frame's update is already running).
    void recordDispatch(CmdBufRecorder &cmdBuf, uint32_t dispatchCount);
    void recordParticleSim(CmdBufRecorder &cmdBuf, uint32_t emitterCount, const mathfu::mat4 &viewMatrix, float deltaSeconds);
    void recordRibbonSim(CmdBufRecorder &cmdBuf, uint32_t ribbonCount, float deltaSeconds);

    bool isEnabled() const { return m_animComputeMaterial != nullptr; }

    // Particle pool accessors (for the GPU particle material's descriptor set)
    const HGBufferVLK &getParticleStatesBuffer() const { return m_particleStates; }
    const HGBufferVLK &getParticleStaticsBuffer() const { return m_particleStatics; }
    const HGBufferVLK &getParticlePropsBuffer() const { return m_particleProps; }
    const HGBufferVLK &getParticlesABuffer() const { return m_particlesA; }
    const HGBufferVLK &getParticlesBBuffer() const { return m_particlesB; }
    const HGBufferVLK &getPartTimesBuffer() const { return m_partTimes; }
    const HGBufferVLK &getRandTableBuffer() const { return m_randTable; }
    const HGBufferVLK &getColorReplBuffer() const { return m_particleColorRepl; }
    const HGBufferVLK &getValuesVec4Buffer() const { return m_valuesVec4; }
    const HGBufferVLK &getValuesFloatBuffer() const { return m_valuesFloat; }

    // Ribbon pool accessors (for the GPU ribbon material's descriptor set)
    const HGBufferVLK &getRibbonStatesBuffer() const { return m_ribbonStates; }
    const HGBufferVLK &getRibbonStaticsBuffer() const { return m_ribbonStatics; }
    const HGBufferVLK &getRibbonPropsBuffer() const { return m_ribbonProps; }
    const HGBufferVLK &getRibbonEdgesBuffer() const { return m_ribbonEdges; }

private:
    HGDeviceVLK m_device;
    Config *m_config;

    // Animation pools
    HGBufferVLK m_animStates;
    HGBufferVLK m_boneStatics;
    HGBufferVLK m_trackHeaders;
    HGBufferVLK m_seqSlices;
    HGBufferVLK m_timestamps;
    HGBufferVLK m_valuesVec4;
    HGBufferVLK m_valuesFloat;
    HGBufferVLK m_seqDurations;
    HGBufferVLK m_globalLoops;
    HGBufferVLK m_dispatchList;

    // Particle pools
    HGBufferVLK m_particleStates;
    HGBufferVLK m_particleStatics;
    HGBufferVLK m_particleProps;
    HGBufferVLK m_particlesA;
    HGBufferVLK m_particlesB;
    HGBufferVLK m_partTimes;
    HGBufferVLK m_randTable;
    HGBufferVLK m_particleColorRepl;
    HGBufferVLK m_emitterDispatchList;

    // Ribbon pools
    HGBufferVLK m_ribbonStates;
    HGBufferVLK m_ribbonStatics;
    HGBufferVLK m_ribbonProps;
    HGBufferVLK m_ribbonEdges;
    HGBufferVLK m_ribbonIndexPool;   // STORAGE | INDEX usage
    HGBufferVLK m_ribbonDispatchList;

    // Not owned: the renderer's bindless bone matrix + placement matrix SSBOs
    HGBufferVLK m_boneMatrix;
    HGBufferVLK m_placementMats;

    std::shared_ptr<ComputeMaterialVLK> m_animComputeMaterial = nullptr;
    std::shared_ptr<ComputeMaterialVLK> m_particleSimMaterial = nullptr;
    std::shared_ptr<ComputeMaterialVLK> m_ribbonSimMaterial = nullptr;

    // Shared static track data, one entry per source model (keyed by the M2Geom
    // pointer). Weak: the set dies when the last referencing object is destroyed.
    // Models that cannot be packed for the GPU path land in m_invalidTrackSetKeys
    // so repeat instances bail without rebuilding the pack.
    std::shared_ptr<M2GpuTrackSetVLK> createTrackSet(const M2GpuTrackPack &pack);
    std::mutex m_trackSetCacheMutex;
    std::unordered_map<uintptr_t, std::weak_ptr<M2GpuTrackSetVLK>> m_trackSetCache;
    std::unordered_set<uintptr_t> m_invalidTrackSetKeys;
};

#endif //AWEBWOWVIEWERCPP_GPUM2ANIMATIONVLK_H
