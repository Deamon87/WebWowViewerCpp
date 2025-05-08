//
// Created by Deamon on 2026-08-18.
//

#include "GpuM2AnimationVLK.h"
#include "../../../gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferRecorder.h"
#include "../../../include/config.h"
#include "../../../engine/managers/particles/particleEmitter.h"

GpuM2AnimationVLK::GpuM2AnimationVLK(const HGDeviceVLK &device, Config *config)
    : m_device(device), m_config(config) {
}

void GpuM2AnimationVLK::setup(std::vector<std::reference_wrapper<HGBufferVLK>> &allBuffers,
                              const HGBufferVLK &boneMatrixBuffer,
                              const HGBufferVLK &placementMatBuffer) {
    m_boneMatrix = boneMatrixBuffer;
    m_placementMats = placementMatBuffer;

    // Initial pool capacities; pools auto-grow (GBufferVLK::resize keeps existing
    // chunk offsets stable and rebinds descriptors through DSBindable on-change).
    m_animStates   = m_device->createSSBOBuffer("M2GpuAnimStates",   4096 * sizeof(GpuM2AnimState), sizeof(GpuM2AnimState));
    m_boneStatics  = m_device->createSSBOBuffer("M2GpuBoneStatics",  (1 << 16) * sizeof(GpuM2BoneStatic), sizeof(GpuM2BoneStatic));
    m_trackHeaders = m_device->createSSBOBuffer("M2GpuTrackHeaders", (1 << 18) * sizeof(GpuM2TrackHeader), sizeof(GpuM2TrackHeader));
    m_seqSlices    = m_device->createSSBOBuffer("M2GpuSeqSlices",    (1 << 18) * sizeof(GpuM2SeqSlice), sizeof(GpuM2SeqSlice));
    m_timestamps   = m_device->createSSBOBuffer("M2GpuTimestamps",   (1 << 20) * sizeof(uint32_t), sizeof(uint32_t));
    m_valuesVec4   = m_device->createSSBOBuffer("M2GpuValuesVec4",   (1 << 19) * sizeof(float) * 4, sizeof(float) * 4);
    m_valuesFloat  = m_device->createSSBOBuffer("M2GpuValuesFloat",  (1 << 18) * sizeof(float), sizeof(float));
    m_seqDurations = m_device->createSSBOBuffer("M2GpuSeqDurations", (1 << 16) * sizeof(uint32_t), sizeof(uint32_t));
    m_globalLoops  = m_device->createSSBOBuffer("M2GpuGlobalLoops",  (1 << 14) * sizeof(uint32_t), sizeof(uint32_t));
    m_dispatchList = m_device->createSSBOBuffer("M2GpuAnimDispatch", (1 << 14) * sizeof(uint32_t), sizeof(uint32_t));

    // Particle pools
    m_particleStates  = m_device->createSSBOBuffer("M2GpuParticleStates",  (1 << 14) * sizeof(GpuParticleState), sizeof(GpuParticleState));
    m_particleStatics = m_device->createSSBOBuffer("M2GpuParticleStatics", (1 << 14) * sizeof(GpuM2ParticleStatic), sizeof(GpuM2ParticleStatic));
    m_particleProps   = m_device->createSSBOBuffer("M2GpuParticleProps",   (1 << 14) * sizeof(GpuM2ParticleFrameProps), sizeof(GpuM2ParticleFrameProps));
    m_particlesA      = m_device->createSSBOBuffer("M2GpuParticlesA",      (1 << 18) * sizeof(GpuParticle), sizeof(GpuParticle));
    m_particlesB      = m_device->createSSBOBuffer("M2GpuParticlesB",      (1 << 18) * sizeof(GpuParticle), sizeof(GpuParticle));
    m_partTimes       = m_device->createSSBOBuffer("M2GpuPartTimes",       (1 << 18) * sizeof(uint32_t), sizeof(uint32_t));
    m_randTable       = m_device->createSSBOBuffer("M2GpuRandTable",       128 * sizeof(float), sizeof(float));
    m_particleColorRepl = m_device->createSSBOBuffer("M2GpuParticleColorRepl", (1 << 12) * sizeof(mathfu::vec4), sizeof(mathfu::vec4));
    m_emitterDispatchList = m_device->createSSBOBuffer("M2GpuEmitterDispatch", (1 << 14) * sizeof(uint32_t), sizeof(uint32_t));

    // Ribbon pools; the index pool doubles as index buffer for the pull-model ribbon meshes
    m_ribbonStates  = m_device->createSSBOBuffer("M2GpuRibbonStates",  (1 << 12) * sizeof(GpuRibbonState), sizeof(GpuRibbonState));
    m_ribbonStatics = m_device->createSSBOBuffer("M2GpuRibbonStatics", (1 << 12) * sizeof(GpuM2RibbonStatic), sizeof(GpuM2RibbonStatic));
    m_ribbonProps   = m_device->createSSBOBuffer("M2GpuRibbonProps",   (1 << 12) * sizeof(GpuM2RibbonFrameProps), sizeof(GpuM2RibbonFrameProps));
    m_ribbonEdges   = m_device->createSSBOBuffer("M2GpuRibbonEdges",   (1 << 16) * sizeof(GpuRibbonEdge), sizeof(GpuRibbonEdge));
    m_ribbonIndexPool = m_device->createSSBOBuffer("M2GpuRibbonIndices", (1 << 17) * sizeof(uint32_t), sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    m_ribbonDispatchList = m_device->createSSBOBuffer("M2GpuRibbonDispatch", (1 << 14) * sizeof(uint32_t), sizeof(uint32_t));

    // The CPU particle twinkle table (process-global; shared with the CPU path)
    ParticleEmitter::ensureRandTableInit();
    m_randTable->uploadData(ParticleEmitter::RandTable, 128 * sizeof(float));

    allBuffers.push_back(m_animStates);
    allBuffers.push_back(m_boneStatics);
    allBuffers.push_back(m_trackHeaders);
    allBuffers.push_back(m_seqSlices);
    allBuffers.push_back(m_timestamps);
    allBuffers.push_back(m_valuesVec4);
    allBuffers.push_back(m_valuesFloat);
    allBuffers.push_back(m_seqDurations);
    allBuffers.push_back(m_globalLoops);
    allBuffers.push_back(m_dispatchList);
    allBuffers.push_back(m_particleStates);
    allBuffers.push_back(m_particleStatics);
    allBuffers.push_back(m_particleProps);
    allBuffers.push_back(m_particlesA);
    allBuffers.push_back(m_particlesB);
    allBuffers.push_back(m_partTimes);
    allBuffers.push_back(m_randTable);
    allBuffers.push_back(m_particleColorRepl);
    allBuffers.push_back(m_emitterDispatchList);
    allBuffers.push_back(m_ribbonStates);
    allBuffers.push_back(m_ribbonStatics);
    allBuffers.push_back(m_ribbonProps);
    allBuffers.push_back(m_ribbonEdges);
    allBuffers.push_back(m_ribbonIndexPool);
    allBuffers.push_back(m_ribbonDispatchList);

    ComputeShaderConfig computeConfig;
    computeConfig.computeShaderFolder = "compute";

    m_animComputeMaterial = ComputeMaterialBuilderVLK::fromShader(m_device, "m2Animation", computeConfig)
        .createDescriptorSet(0, [this](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(0, m_dispatchList)
                .ssbo(1, m_animStates)
                .ssbo(2, m_boneStatics)
                .ssbo(3, m_trackHeaders)
                .ssbo(4, m_seqSlices)
                .ssbo(5, m_timestamps)
                .ssbo(6, m_valuesVec4)
                .ssbo(7, m_valuesFloat)
                .ssbo(8, m_seqDurations)
                .ssbo(9, m_globalLoops)
                .ssbo(10, m_boneMatrix)
                .ssbo(11, m_particleStatics)
                .ssbo(12, m_particleProps)
                .ssbo(13, m_ribbonStatics)
                .ssbo(14, m_ribbonProps)
                .delayUpdate();
        })
        .toMaterial();

    m_particleSimMaterial = ComputeMaterialBuilderVLK::fromShader(m_device, "particleSimulate", computeConfig)
        .createDescriptorSet(0, [this](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(0, m_emitterDispatchList)
                .ssbo(1, m_particleStates)
                .ssbo(2, m_particleStatics)
                .ssbo(3, m_particleProps)
                .ssbo(4, m_particlesA)
                .ssbo(5, m_particlesB)
                .ssbo(6, m_boneMatrix)
                .ssbo(7, m_placementMats)
                .delayUpdate();
        })
        .toMaterial();

    m_ribbonSimMaterial = ComputeMaterialBuilderVLK::fromShader(m_device, "ribbonSimulate", computeConfig)
        .createDescriptorSet(0, [this](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(0, m_ribbonDispatchList)
                .ssbo(1, m_ribbonStates)
                .ssbo(2, m_ribbonStatics)
                .ssbo(3, m_ribbonProps)
                .ssbo(4, m_ribbonEdges)
                .ssbo(5, m_ribbonIndexPool)
                .ssbo(6, m_boneMatrix)
                .ssbo(7, m_placementMats)
                .delayUpdate();
        })
        .toMaterial();
}

namespace {
// Uploads raw bytes into a pool chunk. offsetOut is in pool elements
// (CBufferChunkVLK::getIndex() divides by the pool's record size, so it is
// independent of the chunk's template type).
void allocAndUploadRaw(const HGBufferVLK &pool, const void *data, size_t bytes,
                       std::vector<std::shared_ptr<IBufferVLK>> &keepAlive,
                       int32_t &offsetOut) {
    offsetOut = 0;
    if (bytes == 0) return;

    auto chunk = std::make_shared<CBufferChunkVLK<uint8_t>>(pool, (int)bytes);
    chunk->getSubBuffer()->uploadData(data, (int)bytes);
    offsetOut = (int32_t)chunk->getIndex();
    keepAlive.push_back(chunk->getSubBuffer());
}

// Allocates raw bytes in a pool without uploading (GPU-side state).
void allocRaw(const HGBufferVLK &pool, size_t bytes,
              std::vector<std::shared_ptr<IBufferVLK>> &keepAlive,
              int32_t &offsetOut) {
    offsetOut = 0;
    if (bytes == 0) return;

    auto chunk = std::make_shared<CBufferChunkVLK<uint8_t>>(pool, (int)bytes);
    offsetOut = (int32_t)chunk->getIndex();
    keepAlive.push_back(chunk->getSubBuffer());
}

// Same as allocAndUploadRaw, but the sub-buffer is stored in a dedicated member
// instead of keepAlive so it can be REPLACED later (.anim streaming). The replaced
// chunk's pool range is freed through the deferred deallocation queue, so frames
// still in flight keep reading valid (old) data.
void allocAndUploadRawReplaceable(const HGBufferVLK &pool, const void *data, size_t bytes,
                                  std::shared_ptr<IBufferVLK> &chunkOut,
                                  int32_t &offsetOut) {
    offsetOut = 0;
    chunkOut = nullptr;
    if (bytes == 0) return;

    auto chunk = std::make_shared<CBufferChunkVLK<uint8_t>>(pool, (int)bytes);
    chunk->getSubBuffer()->uploadData(data, (int)bytes);
    offsetOut = (int32_t)chunk->getIndex();
    chunkOut = chunk->getSubBuffer();
}
} // namespace

// Uploads the model-shared static track data into the pools. Called once per
// source model (under m_trackSetCacheMutex).
std::shared_ptr<M2GpuTrackSetVLK> GpuM2AnimationVLK::createTrackSet(const M2GpuTrackPack &pack) {
    auto trackSet = std::make_shared<M2GpuTrackSetVLK>();
    auto &keepAlive = trackSet->m_keepAlive;
    auto &offsets = trackSet->m_offsets;

    allocAndUploadRaw(m_boneStatics, pack.boneStatics.data(), pack.boneStatics.size() * sizeof(GpuM2BoneStatic), keepAlive, offsets.boneStaticOffset);
    allocAndUploadRaw(m_trackHeaders, pack.trackHeaders.data(), pack.trackHeaders.size() * sizeof(GpuM2TrackHeader), keepAlive, offsets.trackHeaderOffset);

    // Replaceable pools (re-allocated on .anim streaming, syncTrackData)
    trackSet->m_seqSlicesPool = m_seqSlices;
    trackSet->m_timestampsPool = m_timestamps;
    trackSet->m_valuesVec4Pool = m_valuesVec4;
    trackSet->m_valuesFloatPool = m_valuesFloat;
    trackSet->m_particleStaticsPool = m_particleStatics;
    trackSet->m_ribbonStaticsPool = m_ribbonStatics;

    allocAndUploadRawReplaceable(m_seqSlices, pack.seqSlices.data(), pack.seqSlices.size() * sizeof(GpuM2SeqSlice), trackSet->m_chunkSeqSlices, offsets.seqSliceOffset);
    allocAndUploadRawReplaceable(m_timestamps, pack.timestamps.data(), pack.timestamps.size() * sizeof(uint32_t), trackSet->m_chunkTimestamps, offsets.timestampOffset);
    allocAndUploadRawReplaceable(m_valuesVec4, pack.valuesVec4.data(), pack.valuesVec4.size() * sizeof(float), trackSet->m_chunkValuesVec4, offsets.valuesVec4Offset);
    allocAndUploadRawReplaceable(m_valuesFloat, pack.valuesFloat.data(), pack.valuesFloat.size() * sizeof(float), trackSet->m_chunkValuesFloat, offsets.valuesFloatOffset);

    allocAndUploadRaw(m_seqDurations, pack.seqDurations.data(), pack.seqDurations.size() * sizeof(uint32_t), keepAlive, offsets.seqDurationOffset);
    allocAndUploadRaw(m_globalLoops, pack.globalLoops.data(), pack.globalLoops.size() * sizeof(uint32_t), keepAlive, offsets.globalLoopOffset);
    allocAndUploadRaw(m_partTimes, pack.partTimes.data(), pack.partTimes.size() * sizeof(uint32_t), keepAlive, offsets.partTimesOffset);

    offsets.ownBoneCount = pack.ownBoneCount;
    offsets.parentBoneCount = pack.parentBoneCount;
    offsets.ownSeqCount = pack.ownSeqCount;
    offsets.parentSeqCount = pack.parentSeqCount;
    offsets.ownGsCount = pack.ownGsCount;
    offsets.parentGsCount = pack.parentGsCount;

    // ---- Particle emitter statics (shared) ----
    const int32_t emitterCount = (int32_t)pack.particleStatics.size();
    if (emitterCount > 0) {
        allocAndUploadRaw(m_particleStatics, pack.particleStatics.data(),
                          emitterCount * sizeof(GpuM2ParticleStatic), keepAlive, offsets.particleStaticsOffset);
    }

    // ---- Ribbon statics (shared) ----
    const int32_t ribbonCount = (int32_t)pack.ribbonStatics.size();
    if (ribbonCount > 0) {
        allocAndUploadRaw(m_ribbonStatics, pack.ribbonStatics.data(),
                          ribbonCount * sizeof(GpuM2RibbonStatic), keepAlive, offsets.ribbonStaticsOffset);
    }

    // Immutable per-model info for per-instance allocation
    auto &modelInfo = trackSet->m_modelInfo;
    modelInfo.ownBoneCount = pack.ownBoneCount;
    modelInfo.particleEmitterCount = emitterCount;
    modelInfo.ribbonCount = ribbonCount;
    modelInfo.particleGpuSupported = pack.particleGpuSupported;
    modelInfo.particleCapacity = pack.particleCapacity;
    modelInfo.particleQuadsPerParticle.resize(emitterCount);
    modelInfo.ribbonEdgesPerSec.resize(ribbonCount);
    modelInfo.ribbonEdgeLifeSpan.resize(ribbonCount);
    for (int32_t i = 0; i < emitterCount; i++) {
        modelInfo.particleQuadsPerParticle[i] = ((pack.particleStatics[i].flags & 0x60000) == 0x60000) ? 2 : 1;
        if (pack.particleStatics[i].particleColorIndex >= 11 && pack.particleStatics[i].particleColorIndex <= 13) {
            modelInfo.needsColorRepl = true;
        }
    }
    for (int32_t i = 0; i < ribbonCount; i++) {
        modelInfo.ribbonEdgesPerSec[i] = pack.ribbonStatics[i].edgesPerSec;
        modelInfo.ribbonEdgeLifeSpan[i] = pack.ribbonStatics[i].edgeLifeSpan;
    }

    // .anim streaming baseline (countGpuLoadedSequences at pack time)
    trackSet->m_loadedSeqCount = pack.loadedSeqCountOwn + pack.loadedSeqCountParent;
    trackSet->m_dataGeneration = 1;

    return trackSet;
}

std::shared_ptr<IM2GpuAnimData> GpuM2AnimationVLK::createAnimData(
    const void *modelKey,
    const std::function<M2GpuTrackPack()> &packBuilder,
    int32_t boneOutOffset,
    int32_t placementMatIndex,
    const std::vector<M2GpuEmitterSeeds> &emitterSeeds,
    const std::vector<int32_t> &emitterRandomizedTextureIndexMasks,
    const std::vector<std::shared_ptr<IBuffer>> &ribbonGpuIndexBuffers) {

    // Get-or-create the model-shared track set. The pack build (engine callback)
    // runs under the cache mutex so concurrent first-inits of the same model
    // collapse into a single upload (M2Object::update runs under tbb::parallel_for).
    std::shared_ptr<M2GpuTrackSetVLK> trackSet;
    {
        std::lock_guard<std::mutex> lock(m_trackSetCacheMutex);
        const uintptr_t key = reinterpret_cast<uintptr_t>(modelKey);
        if (m_invalidTrackSetKeys.count(key) != 0) return nullptr;

        auto it = m_trackSetCache.find(key);
        if (it != m_trackSetCache.end()) {
            trackSet = it->second.lock();
            if (trackSet == nullptr) m_trackSetCache.erase(it);
        }
        if (trackSet == nullptr) {
            M2GpuTrackPack pack = packBuilder();
            if (!pack.valid) {
                m_invalidTrackSetKeys.insert(key);
                return nullptr;
            }
            trackSet = createTrackSet(pack);
            m_trackSetCache.emplace(key, trackSet);
        }
    }

    const auto &modelInfo = trackSet->m_modelInfo; // immutable after creation
    const auto sharedOffsets = trackSet->getSharedOffsets();

    auto result = std::make_shared<M2GpuAnimDataVLK>();
    result->m_trackSet = trackSet;
    result->m_boneOutOffset = boneOutOffset;

    std::vector<std::shared_ptr<IBufferVLK>> keepAlive;

    // ---- Particle emitters (per-object sim state) ----
    const int32_t emitterCount = modelInfo.particleEmitterCount;

    if (emitterCount > 0) {
        int32_t propsOffset = 0;
        allocRaw(m_particleProps, emitterCount * sizeof(GpuM2ParticleFrameProps), keepAlive, propsOffset);
        result->m_particlePropsOffset = propsOffset;
        // Zero-init: the anim pass only writes props for enabled emitters, and the sim
        // reads isEnabled unconditionally — garbage here would spawn garbage particles
        {
            std::vector<GpuM2ParticleFrameProps> zeroProps(emitterCount);
            std::memset(zeroProps.data(), 0, emitterCount * sizeof(GpuM2ParticleFrameProps));
            m_particleProps->uploadDataAtOffset(zeroProps.data(),
                                                (int)(emitterCount * sizeof(GpuM2ParticleFrameProps)),
                                                (size_t)propsOffset * sizeof(GpuM2ParticleFrameProps));
        }

        int32_t statesOffset = 0;
        allocRaw(m_particleStates, emitterCount * sizeof(GpuParticleState), keepAlive, statesOffset);

        // Initialize per-emitter sim states
        {
            std::vector<GpuParticleState> initStates(emitterCount);

            // Total particle storage for this object (GPU-supported emitters only)
            int32_t totalParticles = 0;
            for (int32_t i = 0; i < emitterCount; i++) {
                bool gpuSupported = (i < (int32_t)modelInfo.particleGpuSupported.size())
                                    && modelInfo.particleGpuSupported[i] != 0;
                if (!gpuSupported) continue;
                totalParticles += (i < (int32_t)modelInfo.particleCapacity.size()) ? modelInfo.particleCapacity[i] : 16;
            }

            // Ping-pong particle storage. The two pools have independent offset
            // allocators, so the object's region may start at DIFFERENT element
            // offsets in A and B. Both pool-absolute bases are baked into the sim
            // states — the shaders index the pools absolutely.
            int32_t particlesOffsetA = 0, particlesOffsetB = 0;
            allocRaw(m_particlesA, (size_t)totalParticles * sizeof(GpuParticle), keepAlive, particlesOffsetA);
            allocRaw(m_particlesB, (size_t)totalParticles * sizeof(GpuParticle), keepAlive, particlesOffsetB);

            int32_t particleOffset = 0;

            for (int32_t i = 0; i < emitterCount; i++) {
                GpuParticleState &st = initStates[i];
                std::memset(&st, 0, sizeof(GpuParticleState));

                M2GpuAnimDataVLK::EmitterGpuInfo info;

                bool gpuSupported = (i < (int32_t)modelInfo.particleGpuSupported.size())
                                    && modelInfo.particleGpuSupported[i] != 0;
                if (!gpuSupported) {
                    info.stateIndex = -1;
                    result->m_emitterInfos.push_back(info);
                    continue;
                }

                if (i < (int32_t)emitterSeeds.size()) {
                    st.emitterSeedValue = emitterSeeds[i].emitterSeedValue;
                    st.emitterSeedAccum = emitterSeeds[i].emitterSeedAccum;
                    st.generatorSeedValue = emitterSeeds[i].generatorSeedValue;
                    st.generatorSeedAccum = emitterSeeds[i].generatorSeedAccum;
                }
                if (i < (int32_t)emitterRandomizedTextureIndexMasks.size()) {
                    st.randomizedTextureIndexMask = emitterRandomizedTextureIndexMasks[i];
                }

                st.staticsIndex = sharedOffsets.particleStaticsOffset + i;
                st.propsIndex = propsOffset + i;
                st.particleOffset = particlesOffsetA + particleOffset;
                st.particleOffsetB = particlesOffsetB + particleOffset;
                st.capacity = (i < (int32_t)modelInfo.particleCapacity.size()) ? modelInfo.particleCapacity[i] : 16;
                st.placementMatIndex = placementMatIndex;
                st.boneOutOffset = boneOutOffset;
                st.curBuf = 0;
                st.particleCount = 0;

                info.stateIndex = statesOffset + i;
                info.particleOffset = st.particleOffset;
                info.capacity = st.capacity;
                info.quadsPerParticle = modelInfo.particleQuadsPerParticle[i];
                result->m_emitterInfos.push_back(info);

                particleOffset += st.capacity;
            }

            // Upload initial states into the states chunk allocated above
            m_particleStates->uploadDataAtOffset(initStates.data(),
                                                 (int)(emitterCount * sizeof(GpuParticleState)),
                                                 (size_t)statesOffset * sizeof(GpuParticleState));
        }
    }

    // ---- Ribbons (per-object sim state) ----
    const int32_t ribbonCount = modelInfo.ribbonCount;

    if (ribbonCount > 0) {
        int32_t ribbonPropsOffset = 0;
        allocRaw(m_ribbonProps, ribbonCount * sizeof(GpuM2RibbonFrameProps), keepAlive, ribbonPropsOffset);
        result->m_ribbonPropsOffset = ribbonPropsOffset;
        // Zero-init (dataEnabled=0 until the anim pass writes real props)
        {
            std::vector<GpuM2RibbonFrameProps> zeroProps(ribbonCount);
            std::memset(zeroProps.data(), 0, ribbonCount * sizeof(GpuM2RibbonFrameProps));
            m_ribbonProps->uploadDataAtOffset(zeroProps.data(),
                                              (int)(ribbonCount * sizeof(GpuM2RibbonFrameProps)),
                                              (size_t)ribbonPropsOffset * sizeof(GpuM2RibbonFrameProps));
        }

        int32_t ribbonStatesOffset = 0;
        allocRaw(m_ribbonStates, ribbonCount * sizeof(GpuRibbonState), keepAlive, ribbonStatesOffset);

        int32_t edgesOffset = 0;
        {
            std::vector<GpuRibbonState> initStates(ribbonCount);

            // Per-ribbon edge counts + total (CRibbonEmitter::Initialize:
            // edgeCount = ceil(max(ceil(lifeSpan*perSec)+1+1, 0)))
            std::vector<int32_t> edgeCounts(ribbonCount, 0);
            int32_t totalEdges = 0;
            for (int32_t i = 0; i < ribbonCount; i++) {
                float edgesPerSec = modelInfo.ribbonEdgesPerSec[i];
                float edgeLifeSpan = modelInfo.ribbonEdgeLifeSpan[i];
                edgeCounts[i] = (int32_t)ceilf(fmaxf(ceilf(edgeLifeSpan * edgesPerSec) + 1.0f + 1.0f, 0.0f));
                totalEdges += edgeCounts[i];
            }

            // Edge ring storage: the pool-absolute base is baked into the sim
            // states — the shaders index the edges pool absolutely.
            int32_t edgesPoolOffset = 0;
            allocRaw(m_ribbonEdges, (size_t)totalEdges * sizeof(GpuRibbonEdge), keepAlive, edgesPoolOffset);

            for (int32_t i = 0; i < ribbonCount; i++) {
                GpuRibbonState &rst = initStates[i];
                std::memset(&rst, 0, sizeof(GpuRibbonState));

                int32_t edgeCount = edgeCounts[i];

                rst.staticsIndex = sharedOffsets.ribbonStaticsOffset + i;
                rst.propsIndex = ribbonPropsOffset + i;
                rst.edgesOffset = edgesPoolOffset + edgesOffset;
                rst.edgeCount = edgeCount;
                rst.placementMatIndex = placementMatIndex;
                rst.boneOutOffset = boneOutOffset;
                // The ribbon's GPU index buffer chunk (created at load time)
                auto iboVlk = (i < (int32_t)ribbonGpuIndexBuffers.size() && ribbonGpuIndexBuffers[i] != nullptr)
                    ? std::dynamic_pointer_cast<IBufferVLK>(ribbonGpuIndexBuffers[i]) : nullptr;
                rst.iboOffset = iboVlk ? (int32_t)(iboVlk->getOffset() / sizeof(uint32_t)) : 0;
                rst.iboCount = 4 * edgeCount;

                M2GpuAnimDataVLK::RibbonGpuInfo info;
                info.stateIndex = ribbonStatesOffset + i;
                info.edgesOffset = rst.edgesOffset;
                info.edgeCount = edgeCount;
                result->m_ribbonInfos.push_back(info);

                edgesOffset += edgeCount;
            }

            m_ribbonStates->uploadDataAtOffset(initStates.data(),
                                               (int)(ribbonCount * sizeof(GpuRibbonState)),
                                               (size_t)ribbonStatesOffset * sizeof(GpuRibbonState));
        }
    }

    // anim-state slot
    auto stateChunk = std::make_shared<CBufferChunkVLK<GpuM2AnimState>>(m_animStates);
    result->m_stateChunk = stateChunk;
    keepAlive.push_back(stateChunk->getSubBuffer());

    // Identity-initialize the object's bone region so frames skipped by the deferred
    // (.anim) loading gate keep valid matrices (same initial state as the CPU path)
    if (modelInfo.ownBoneCount > 0) {
        std::vector<mathfu::mat4> identityBones(modelInfo.ownBoneCount, mathfu::mat4::Identity());
        m_boneMatrix->uploadDataAtOffset(identityBones.data(),
                                         (int)(identityBones.size() * sizeof(mathfu::mat4)),
                                         (size_t)boneOutOffset * sizeof(mathfu::mat4));
    }

    // ParticleColor.dbc replacement chunk (9 vec4) when any emitter references it
    if (modelInfo.needsColorRepl) {
        allocRaw(m_particleColorRepl, 9 * sizeof(mathfu::vec4), keepAlive, result->m_colorReplOffset);
        result->m_colorReplChunk = keepAlive.back();
    }

    // Hold the per-object chunks alive as long as the object's anim data lives
    result->m_keepAlive = std::move(keepAlive);

    return result;
}

void M2GpuTrackSetVLK::syncTrackData(int32_t loadedSeqCount,
                                     const std::function<M2GpuTrackPack()> &packBuilder) {
    // Cheap unlocked precheck; the count only grows, so this is stable
    if (loadedSeqCount <= m_loadedSeqCount.load(std::memory_order_acquire)) return;

    std::lock_guard<std::mutex> lock(m_mutex);
    // Another instance of the same model may have synced while we waited
    if (loadedSeqCount <= m_loadedSeqCount.load(std::memory_order_relaxed)) return;

    M2GpuTrackPack pack = packBuilder();
    if (!pack.valid) return; // shouldn't happen (same model); keep old data

    // Replace the four track-value chunks (they only grow as lazy .anim sequences
    // stream in). Old chunks are freed through the deferred deallocation queue, so
    // frames still in flight keep reading valid data.
    allocAndUploadRawReplaceable(m_seqSlicesPool, pack.seqSlices.data(),
                                 pack.seqSlices.size() * sizeof(GpuM2SeqSlice),
                                 m_chunkSeqSlices, m_offsets.seqSliceOffset);
    allocAndUploadRawReplaceable(m_timestampsPool, pack.timestamps.data(),
                                 pack.timestamps.size() * sizeof(uint32_t),
                                 m_chunkTimestamps, m_offsets.timestampOffset);
    allocAndUploadRawReplaceable(m_valuesVec4Pool, pack.valuesVec4.data(),
                                 pack.valuesVec4.size() * sizeof(float),
                                 m_chunkValuesVec4, m_offsets.valuesVec4Offset);
    allocAndUploadRawReplaceable(m_valuesFloatPool, pack.valuesFloat.data(),
                                 pack.valuesFloat.size() * sizeof(float),
                                 m_chunkValuesFloat, m_offsets.valuesFloatOffset);

    // Particle/ribbon statics embed slice offsets into the value pools, so their
    // content moves with every repack. Re-upload in place (count is fixed at
    // creation, so size and pool offset are unchanged) to keep the live sim
    // states' staticsIndex valid.
    if (!pack.particleStatics.empty() && m_modelInfo.particleEmitterCount > 0) {
        m_particleStaticsPool->uploadDataAtOffset(pack.particleStatics.data(),
            (int)(pack.particleStatics.size() * sizeof(GpuM2ParticleStatic)),
            (size_t)m_offsets.particleStaticsOffset * sizeof(GpuM2ParticleStatic));
    }
    if (!pack.ribbonStatics.empty() && m_modelInfo.ribbonCount > 0) {
        m_ribbonStaticsPool->uploadDataAtOffset(pack.ribbonStatics.data(),
            (int)(pack.ribbonStatics.size() * sizeof(GpuM2RibbonStatic)),
            (size_t)m_offsets.ribbonStaticsOffset * sizeof(GpuM2RibbonStatic));
    }

    m_loadedSeqCount.store(pack.loadedSeqCountOwn + pack.loadedSeqCountParent, std::memory_order_release);
    m_dataGeneration.fetch_add(1, std::memory_order_release);
}

void GpuM2AnimationVLK::updateDispatchList(const std::vector<uint32_t> &stateIndices) {
    if (stateIndices.empty()) return;

    m_dispatchList->uploadData(stateIndices.data(), (int)(stateIndices.size() * sizeof(uint32_t)));
}

void GpuM2AnimationVLK::updateEmitterDispatchList(const std::vector<uint32_t> &emitterStateIndices) {
    if (emitterStateIndices.empty()) return;

    m_emitterDispatchList->uploadData(emitterStateIndices.data(), (int)(emitterStateIndices.size() * sizeof(uint32_t)));
}

std::shared_ptr<IBuffer> GpuM2AnimationVLK::createRibbonGpuIndexBuffer(int32_t edgeCount) {
    // 4 * edgeCount uint16 indices per ribbon, stored as uint32 pairs
    auto chunk = std::make_shared<CBufferChunkVLK<uint8_t>>(m_ribbonIndexPool, edgeCount * 4 * (int)sizeof(uint16_t));
    return chunk->getSubBuffer();
}

void GpuM2AnimationVLK::updateRibbonDispatchList(const std::vector<uint32_t> &ribbonStateIndices) {
    if (ribbonStateIndices.empty()) return;

    m_ribbonDispatchList->uploadData(ribbonStateIndices.data(), (int)(ribbonStateIndices.size() * sizeof(uint32_t)));
}

void GpuM2AnimationVLK::recordDispatch(CmdBufRecorder &cmdBuf, uint32_t dispatchCount) {
    if (!m_animComputeMaterial || dispatchCount == 0) return;

    ZoneScoped;
    VkZone(cmdBuf, "M2 GPU Animation Dispatch");
    auto debugHelper = cmdBuf.beginDebugLabel("M2 GPU Animation Dispatch", {0.2f, 0.5f, 0.2f});

    cmdBuf.bindComputePipeline(m_animComputeMaterial->pipeline);
    cmdBuf.bindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, m_animComputeMaterial->descriptorSets);

    cmdBuf.dispatch(dispatchCount, 1, 1);

    // Bone matrices: compute write -> vertex shader read (and compute read for the
    // particle sim). Emitter props: written here, read by the particle sim.
    // Ribbon props: written here, read by the ribbon sim.
    std::vector<VkBufferMemoryBarrier> barriers = {{
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_SHADER_WRITE_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        m_boneMatrix->getGPUBuffer(),
        0,
        m_boneMatrix->getGPUBufferSize()
    }, {
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_SHADER_WRITE_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        m_particleProps->getGPUBuffer(),
        0,
        m_particleProps->getGPUBufferSize()
    }, {
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_SHADER_WRITE_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        m_ribbonProps->getGPUBuffer(),
        0,
        m_ribbonProps->getGPUBufferSize()
    }};

    cmdBuf.recordPipelineBufferBarrier(
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
        barriers
    );
}

void GpuM2AnimationVLK::recordParticleSim(CmdBufRecorder &cmdBuf, uint32_t emitterCount, const mathfu::mat4 &viewMatrix, float deltaSeconds) {
    if (!m_particleSimMaterial || emitterCount == 0) return;

    ZoneScoped;
    VkZone(cmdBuf, "M2 GPU Particle Sim Dispatch");
    auto debugHelper = cmdBuf.beginDebugLabel("M2 GPU Particle Sim Dispatch", {0.5f, 0.5f, 0.2f});

    struct {
        mathfu::mat4 viewMatrix;
        float deltaSeconds;
        float pad0;
        float pad1;
        float pad2;
    } pushConsts;
    pushConsts.viewMatrix = viewMatrix;
    pushConsts.deltaSeconds = deltaSeconds;
    pushConsts.pad0 = pushConsts.pad1 = pushConsts.pad2 = 0.0f;

    cmdBuf.bindComputePipeline(m_particleSimMaterial->pipeline);
    cmdBuf.bindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, m_particleSimMaterial->descriptorSets);
    cmdBuf.pushConstants(
        m_particleSimMaterial->pipeline->getLayout()->getLayout(),
        VK_SHADER_STAGE_COMPUTE_BIT,
        0,
        sizeof(pushConsts),
        &pushConsts
    );

    cmdBuf.dispatch(emitterCount, 1, 1);

    // Particle pools: compute write -> vertex shader read (pull-model VS) and
    // compute read/write (next frame's sim)
    std::vector<VkBufferMemoryBarrier> barriers;
    for (auto &buf : {m_particleStates, m_particlesA, m_particlesB}) {
        barriers.push_back({
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            nullptr,
            VK_ACCESS_SHADER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            buf->getGPUBuffer(),
            0,
            buf->getGPUBufferSize()
        });
    }

    cmdBuf.recordPipelineBufferBarrier(
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
        barriers
    );
}

void GpuM2AnimationVLK::recordRibbonSim(CmdBufRecorder &cmdBuf, uint32_t ribbonCount, float deltaSeconds) {
    if (!m_ribbonSimMaterial || ribbonCount == 0) return;

    ZoneScoped;
    VkZone(cmdBuf, "M2 GPU Ribbon Sim Dispatch");
    auto debugHelper = cmdBuf.beginDebugLabel("M2 GPU Ribbon Sim Dispatch", {0.8f, 0.5f, 0.2f});

    struct {
        float deltaSeconds;
        float pad0;
        float pad1;
        float pad2;
    } pushConsts;
    pushConsts.deltaSeconds = deltaSeconds;
    pushConsts.pad0 = pushConsts.pad1 = pushConsts.pad2 = 0.0f;

    cmdBuf.bindComputePipeline(m_ribbonSimMaterial->pipeline);
    cmdBuf.bindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, m_ribbonSimMaterial->descriptorSets);
    cmdBuf.pushConstants(
        m_ribbonSimMaterial->pipeline->getLayout()->getLayout(),
        VK_SHADER_STAGE_COMPUTE_BIT,
        0,
        sizeof(pushConsts),
        &pushConsts
    );

    cmdBuf.dispatch(ribbonCount, 1, 1);

    // Ribbon states + edges + index pool: compute write -> vertex shader / index input read
    std::vector<VkBufferMemoryBarrier> barriers;
    for (auto &buf : {m_ribbonStates, m_ribbonEdges}) {
        barriers.push_back({
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            nullptr,
            VK_ACCESS_SHADER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            buf->getGPUBuffer(),
            0,
            buf->getGPUBufferSize()
        });
    }
    barriers.push_back({
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_SHADER_WRITE_BIT,
        VK_ACCESS_INDEX_READ_BIT,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        m_ribbonIndexPool->getGPUBuffer(),
        0,
        m_ribbonIndexPool->getGPUBufferSize()
    });

    cmdBuf.recordPipelineBufferBarrier(
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        barriers
    );
}
