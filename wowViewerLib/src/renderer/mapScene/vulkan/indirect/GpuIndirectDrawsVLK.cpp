#include "GpuIndirectDrawsVLK.h"

#include <iostream>

#include "tbb/tbb.h"

#include "../../MapScenePlan.h"
#include "../../../frame/FrameProfile.h"
#include "../../../../engine/objects/m2/m2Object.h"
#include "../../../../engine/objects/wmo/wmoGroupObject.h"
#include "../../../../gapi/interface/FrameContext.h"
#include "../../../../gapi/vulkan/meshes/GMeshVLK.h"
#include "../../../../gapi/vulkan/materials/ISimpleMaterialVLK.h"
#include "../../../../include/config.h"

static uint32_t toIndex(M2ObjId id) { return static_cast<uint32_t>(static_cast<uintptr_t>(id)); }
static uint32_t toIndex(GMeshId id) { return static_cast<uint32_t>(static_cast<uintptr_t>(id)); }

GpuIndirectDrawsVLK::GpuIndirectDrawsVLK(const HGDeviceVLK &device, Config *config)
    : m_device(device), m_config(config) {
    m_multiDrawIndirectSupported = device->supportsMultiDrawIndirect();
    m_staticCmdMirror.resize(STATIC_CMD_CAPACITY);
    m_wmoAabbMirror.resize(m_wmoGroupCullSlots);
    for (auto &slot : m_frameSlots) {
        slot = std::make_shared<FrameDrawData>();
    }
}

GpuIndirectDrawsVLK::~GpuIndirectDrawsVLK() = default;

void GpuIndirectDrawsVLK::setup(std::vector<std::reference_wrapper<HGBufferVLK>> &allBuffers) {
    static constexpr size_t AABB_RECORD_SIZE = sizeof(GPUAabb);
    const size_t totalCullCapacity = (size_t)m_m2CullCapacity + m_wmoGroupCullSlots;

    m_aabbBuffer = m_device->createSSBOBuffer("CullAABBs", totalCullCapacity * AABB_RECORD_SIZE, AABB_RECORD_SIZE);
    m_visibilityBuffer = m_device->createSSBOBuffer("CullVisibility", totalCullCapacity * sizeof(uint32_t), sizeof(uint32_t));
    m_classificationBuffer = m_device->createSSBOBuffer("LiquidClassification", totalCullCapacity * sizeof(uint32_t), sizeof(uint32_t));
    m_indirectDrawBuffer = m_device->createSSBOBuffer("IndirectDrawBuffer",
        (size_t)m_maxDrawCommands * sizeof(VkDrawIndexedIndirectCommand),
        sizeof(VkDrawIndexedIndirectCommand),
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
    m_staticCmdBuffer = m_device->createSSBOBuffer("StaticMeshCommands",
        (size_t)m_staticCmdCapacity * sizeof(VkDrawIndexedIndirectCommand),
        sizeof(VkDrawIndexedIndirectCommand));
    m_activeListBuffer = m_device->createSSBOBuffer("ActiveDrawList",
        (size_t)m_maxDrawCommands * sizeof(GPUActiveDrawEntry),
        sizeof(GPUActiveDrawEntry));

    allBuffers.push_back(m_aabbBuffer);
    allBuffers.push_back(m_activeListBuffer);
    allBuffers.push_back(m_staticCmdBuffer);
    allBuffers.push_back(m_indirectDrawBuffer);
    // These two are GPU-written and never upload from the CPU, but they still must be in
    // the upload list: getSubmitRecords() is the only place where a resize()d buffer gets
    // its GPU-side reallocation (content copy + descriptor rebind through on-change).
    allBuffers.push_back(m_visibilityBuffer);
    allBuffers.push_back(m_classificationBuffer);

    ComputeShaderConfig cullConfig;
    cullConfig.computeShaderFolder = "compute";

    // Frustum cull: AABB → visibility buffer
    m_cullComputeMaterial = ComputeMaterialBuilderVLK::fromShader(m_device, "frustumCull", cullConfig)
        .createDescriptorSet(0, [this](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(0, m_aabbBuffer)
                .ssbo(1, m_visibilityBuffer)
                .delayUpdate();
        })
        .toMaterial();

    // Gather + cull: active list + static templates + visibility/classification → compact indirect commands
    m_cullIndirectMaterial = ComputeMaterialBuilderVLK::fromShader(m_device, "cullIndirectCommands", cullConfig)
        .createDescriptorSet(0, [this](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(0, m_activeListBuffer)
                .ssbo(1, m_visibilityBuffer)
                .ssbo(2, m_indirectDrawBuffer)
                .ssbo(3, m_classificationBuffer)
                .ssbo(4, m_staticCmdBuffer)
                .delayUpdate();
        })
        .toMaterial();

    // Liquid classification compute material
    m_classifyComputeMaterial = ComputeMaterialBuilderVLK::fromShader(m_device, "liquidClassify", cullConfig)
        .toMaterial();
}

// ----------------------
// Mesh registration (static draw-command templates)
// ----------------------

VkDrawIndexedIndirectCommand GpuIndirectDrawsVLK::buildStaticCommand(GMeshVLK *mesh) {
    VkDrawIndexedIndirectCommand cmd = {};
    cmd.indexCount = mesh->end();
    cmd.instanceCount = 1;
    cmd.firstIndex = mesh->start() / 2;
    cmd.vertexOffset = mesh->vertexStart;
    cmd.firstInstance = (mesh->instanceIndex != -1) ? (uint32_t)mesh->instanceIndex : 0;
    return cmd;
}

void GpuIndirectDrawsVLK::registerMesh(const std::shared_ptr<GMeshVLK> &mesh) {
    if (!mesh) return;

    uint32_t meshId = toIndex(mesh->getObjectId());
    std::lock_guard<std::mutex> lock(m_staticCmdMutex);

    if (meshId >= m_staticCmdCapacity && !growStaticCmdStorage(meshId)) {
        static bool logged = false;
        if (!logged) {
            logged = true;
            std::cout << "GpuIndirectDrawsVLK: meshId " << meshId << " exceeds STATIC_CMD_MAX_CAPACITY, mesh skipped" << std::endl;
        }
        return;
    }

    m_staticCmdMirror[meshId] = buildStaticCommand(mesh.get());
    m_staticDirtyMin = std::min(m_staticDirtyMin, meshId);
    m_staticDirtyMax = std::max(m_staticDirtyMax, meshId);
}

// Doubles the static command storage until meshId fits. The GPU buffer itself is
// reallocated lazily in GBufferVLK::getSubmitRecords: the old content is copied into
// the new buffer and the descriptor sets bound to it re-bind through the buffer's
// on-handle-change callback, so no descriptor work is needed here.
bool GpuIndirectDrawsVLK::growStaticCmdStorage(uint32_t meshId) {
    uint64_t newCapacity = m_staticCmdCapacity;
    while (newCapacity <= meshId) {
        newCapacity *= 2;
    }
    if (newCapacity > STATIC_CMD_MAX_CAPACITY) return false;

    m_staticCmdMirror.resize(newCapacity);
    if (m_staticCmdBuffer) {
        m_staticCmdBuffer->resize((int)(newCapacity * sizeof(VkDrawIndexedIndirectCommand)));
    }
    m_staticCmdCapacity = (uint32_t)newCapacity;

    std::cout << "GpuIndirectDrawsVLK: grew static mesh command storage to "
              << m_staticCmdCapacity << " meshes" << std::endl;
    return true;
}

void GpuIndirectDrawsVLK::syncStaticDrawCommands() {
    std::lock_guard<std::mutex> lock(m_staticCmdMutex);

    if (m_staticDirtyMin > m_staticDirtyMax) return; // nothing dirty

    uint32_t count = m_staticDirtyMax - m_staticDirtyMin + 1;
    m_staticCmdBuffer->uploadDataAtOffset(
        m_staticCmdMirror.data() + m_staticDirtyMin,
        count * sizeof(VkDrawIndexedIndirectCommand),
        m_staticDirtyMin * sizeof(VkDrawIndexedIndirectCommand));

    m_staticDirtyMin = UINT32_MAX;
    m_staticDirtyMax = 0;
}

// ----------------------
// Per-frame build
// ----------------------

std::shared_ptr<GpuIndirectDrawsVLK::FrameDrawData> GpuIndirectDrawsVLK::buildFrameDraws(const std::shared_ptr<MapRenderPlan> &framePlan) {
    ZoneScoped;

    auto frameData = m_frameSlots[m_buildCounter++ % m_frameSlots.size()];
    frameData->opaqueDraws.clear();
    frameData->transparentDraws.clear();

    // Static/AABB data sync (dirty ranges only)
    syncStaticDrawCommands();
    syncM2Aabbs();
    collectWmoGroups(framePlan);

    frameData->totalCullCount = m_m2CullCapacity + m_wmoGroupHighWater;
    frameData->m2AabbCount = m_syncedM2AabbCount;
    frameData->wmoGroupBase = m_m2CullCapacity;
    frameData->wmoGroupCount = m_wmoGroupHighWater;

    collectMeshDraws(framePlan, *frameData);
    uploadActiveList(*frameData);

    return frameData;
}

void GpuIndirectDrawsVLK::syncM2Aabbs() {
    ZoneScoped;
    if (!m2Factory->isComponentDirty<1>()) return;

    size_t capacity = m2Factory->getComponentCapacity<1>();
    if (capacity > m_m2CullCapacity) {
        growM2CullRegion((uint32_t)capacity);
    }
    if (capacity == 0) return;

    m_m2AabbScratch.resize(capacity);
    m2Factory->copyComponentToBuffer<1>(m_m2AabbScratch.data(), capacity);

    m_gpuAabbScratch.resize(capacity);
    for (size_t i = 0; i < capacity; i++) {
        auto &src = m_m2AabbScratch[i];
        auto &dst = m_gpuAabbScratch[i];
        dst.min = mathfu::vec4_packed(mathfu::vec4(src.min.x, src.min.y, src.min.z, 1.0f));
        dst.max = mathfu::vec4_packed(mathfu::vec4(src.max.x, src.max.y, src.max.z, 1.0f));
    }

    m_aabbBuffer->uploadData(m_gpuAabbScratch.data(), (int)(capacity * sizeof(GPUAabb)));
    m2Factory->clearComponentDirty<1>();
    m_syncedM2AabbCount = (uint32_t)capacity;
}

void GpuIndirectDrawsVLK::resizeCullBuffers() {
    const size_t totalCullCapacity = (size_t)m_m2CullCapacity + m_wmoGroupCullSlots;
    m_aabbBuffer->resize((int)(totalCullCapacity * sizeof(GPUAabb)));
    m_visibilityBuffer->resize((int)(totalCullCapacity * sizeof(uint32_t)));
    m_classificationBuffer->resize((int)(totalCullCapacity * sizeof(uint32_t)));
}

void GpuIndirectDrawsVLK::growM2CullRegion(uint32_t requiredCapacity) {
    uint64_t newCapacity = m_m2CullCapacity;
    while (newCapacity < requiredCapacity) {
        newCapacity *= 2;
    }
    m_m2CullCapacity = (uint32_t)newCapacity;
    resizeCullBuffers();

    // The WMO region moved: re-upload the whole mirror at the new base.
    if (m_wmoGroupHighWater > 0) {
        m_aabbBuffer->uploadDataAtOffset(
            m_wmoAabbMirror.data(),
            m_wmoGroupHighWater * sizeof(GPUAabb),
            (size_t)m_m2CullCapacity * sizeof(GPUAabb));
    }

    std::cout << "GpuIndirectDrawsVLK: grew M2 cull region to "
              << m_m2CullCapacity << " objects" << std::endl;
}

void GpuIndirectDrawsVLK::growWmoGroupSlots() {
    m_wmoGroupCullSlots *= 2;
    m_wmoAabbMirror.resize(m_wmoGroupCullSlots);
    resizeCullBuffers();

    std::cout << "GpuIndirectDrawsVLK: grew WMO group cull slots to "
              << m_wmoGroupCullSlots << " groups" << std::endl;
}

void GpuIndirectDrawsVLK::growDrawCommandStorage(uint32_t requiredCount) {
    uint64_t newCapacity = m_maxDrawCommands;
    while (newCapacity < requiredCount) {
        newCapacity *= 2;
    }
    m_maxDrawCommands = (uint32_t)newCapacity;

    m_indirectDrawBuffer->resize((int)(newCapacity * sizeof(VkDrawIndexedIndirectCommand)));
    m_activeListBuffer->resize((int)(newCapacity * sizeof(GPUActiveDrawEntry)));

    std::cout << "GpuIndirectDrawsVLK: grew draw command storage to "
              << m_maxDrawCommands << " draws" << std::endl;
}

void GpuIndirectDrawsVLK::collectWmoGroups(const std::shared_ptr<MapRenderPlan> &framePlan) {
    ZoneScoped;
    m_drawnWmoGroups.clear();
    const uint32_t thisFrame = m_frameCounter++;

    auto addWmoGroupsFromView = [&](const std::shared_ptr<GeneralView> &view) {
        if (!view) return;
        for (auto &wmoGroup : view->wmoGroupArray.getToDraw()) {
            auto *ptr = wmoGroup.get();

            auto &entry = m_wmoGroupRegistry[ptr];
            if (entry.slot == UINT32_MAX) {
                // First sight: assign a persistent cull slot
                if (!m_wmoGroupFreeSlots.empty()) {
                    entry.slot = m_wmoGroupFreeSlots.back();
                    m_wmoGroupFreeSlots.pop_back();
                } else {
                    if (m_wmoGroupHighWater >= m_wmoGroupCullSlots) {
                        growWmoGroupSlots();
                    }
                    entry.slot = m_wmoGroupHighWater++;
                }
                entry.weak = wmoGroup;
                entry.aabbWritten = false;
            }

            // Group AABBs never change — upload once, when the group has loaded
            if (!entry.aabbWritten && ptr->getIsLoaded()) {
                auto aabb = ptr->getWorldAABB();
                auto &dst = m_wmoAabbMirror[entry.slot];
                dst.min = mathfu::vec4_packed(mathfu::vec4(aabb.min.x, aabb.min.y, aabb.min.z, 1.0f));
                dst.max = mathfu::vec4_packed(mathfu::vec4(aabb.max.x, aabb.max.y, aabb.max.z, 1.0f));
                entry.aabbWritten = true;
                m_wmoAabbDirtyMin = std::min(m_wmoAabbDirtyMin, entry.slot);
                m_wmoAabbDirtyMax = std::max(m_wmoAabbDirtyMax, entry.slot);
            }

            if (entry.lastSeenFrame != thisFrame) {
                entry.lastSeenFrame = thisFrame;
                m_drawnWmoGroups.emplace_back(ptr, entry.slot);
            }
        }
    };

    addWmoGroupsFromView(framePlan->viewsHolder.getExterior());
    for (auto &view : framePlan->viewsHolder.getInteriorViews()) {
        addWmoGroupsFromView(view);
    }

    // Sweep slots of dead groups occasionally (groups die with their WMO)
    if ((thisFrame % 300) == 0) {
        for (auto it = m_wmoGroupRegistry.begin(); it != m_wmoGroupRegistry.end();) {
            if (it->second.weak.expired()) {
                m_wmoGroupFreeSlots.push_back(it->second.slot);
                it = m_wmoGroupRegistry.erase(it);
            } else {
                ++it;
            }
        }
    }

    // Flush the WMO-group AABB dirty span (region starts at m_m2CullCapacity)
    if (m_wmoAabbDirtyMin <= m_wmoAabbDirtyMax) {
        uint32_t count = m_wmoAabbDirtyMax - m_wmoAabbDirtyMin + 1;
        m_aabbBuffer->uploadDataAtOffset(
            m_wmoAabbMirror.data() + m_wmoAabbDirtyMin,
            count * sizeof(GPUAabb),
            ((size_t)m_m2CullCapacity + m_wmoAabbDirtyMin) * sizeof(GPUAabb));
        m_wmoAabbDirtyMin = UINT32_MAX;
        m_wmoAabbDirtyMax = 0;
    }
}

void GpuIndirectDrawsVLK::fillDrawEntry(GMeshVLK *mesh, uint32_t cullObjectId, IndirectDrawEntry &entry) {
    entry.meshId = toIndex(mesh->getObjectId());
    entry.cullObjectId = cullObjectId;
    entry.dynFirstIndex = 0;
    entry.dynIndexCount = 0;
    entry.material = mesh->material().get();
    entry.matId = mesh->material()->getMaterialId();
    entry.priorityPlane = mesh->priorityPlane();
    entry.vertexBindingsShared = mesh->bindings();
#ifdef DEBUG_MESH_NAMES
    entry.debugName = mesh->debugName();
#endif
}

void GpuIndirectDrawsVLK::collectM2Meshes(M2Object *m2Object, uint32_t cullObjectId,
                                          std::vector<IndirectDrawEntry> &opaqueDraws,
                                          std::vector<IndirectDrawEntry> &transpDraws,
                                          std::vector<M2GroupHeader> &groupHeaders) {
    if (!m2Object->getGetIsLoaded()) return;

    M2GroupHeader header;
    header.sortDistance = m2Object->getCurrentDistance();
    header.transpBegin = transpDraws.size();
    const uint32_t opaqueBegin = opaqueDraws.size();

    // The M2 serves its meshes in render order: transparent ones are already sorted
    // within the object (M2Object::forEachVisibleMeshSorted) — keep that order here.
    m2Object->forEachVisibleMeshSorted([&](const HGSortableMesh &mesh, bool hasDynamicDrawParams) {
        auto *vlkMesh = static_cast<GMeshVLK*>(mesh.get());
        IndirectDrawEntry *e;
        if (mesh->getIsTransparent()) {
            e = &transpDraws.emplace_back();
        } else {
            e = &opaqueDraws.emplace_back();
        }
        fillDrawEntry(vlkMesh, cullObjectId, *e);
        // Particles/ribbons patch start/end per frame — carry the live values as overrides
        if (hasDynamicDrawParams) {
            e->dynFirstIndex = mesh->start() / 2;
            e->dynIndexCount = mesh->end();
        }
    });

    header.transpEnd = transpDraws.size();
    if (header.transpEnd > header.transpBegin || opaqueDraws.size() > opaqueBegin) {
        groupHeaders.push_back(header);
    }
}

void GpuIndirectDrawsVLK::collectMeshDraws(const std::shared_ptr<MapRenderPlan> &framePlan, FrameDrawData &frameData) {
    ZoneScoped;

    m_m2GroupHeaders.clear();
    m_transpScratch.clear();

    if (m_config->renderM2) {
        auto &m2ToDraw = framePlan->m2Array.getDrawn();
        const size_t m2Count = m2ToDraw.size();
        const auto threadsAvailable = m_config->hardwareThreadCount();

        if (m2Count >= 64 && threadsAvailable > 1) {
            // Parallel collection: per-chunk outputs written by exactly one worker,
            // merged in chunk order — deterministic, identical to the serial order.
            struct M2ChunkOut {
                std::vector<IndirectDrawEntry> opaque;
                std::vector<IndirectDrawEntry> transp;
                std::vector<M2GroupHeader> headers;
            };
            const size_t chunkSize = std::max<size_t>(m2Count / (2 * threadsAvailable), 1);
            const size_t chunkCount = (m2Count + chunkSize - 1) / chunkSize;
            std::vector<M2ChunkOut> chunkOuts(chunkCount);
            const auto processingFrame = FrameContext::getCurrentProcessingFrameNumber();

            {
                ZoneScopedN("M2 collect (parallel)");
                oneapi::tbb::task_arena arena(std::min<uint32_t>(threadsAvailable, 16), 1);
                arena.execute([&] {
                    tbb::parallel_for(tbb::blocked_range<size_t>(0, chunkCount, 1),
                                      [&](const tbb::blocked_range<size_t> &r) {
                        FrameContext::setCurrentProcessingFrameNumber(processingFrame);
                        for (size_t c = r.begin(); c != r.end(); ++c) {
                            auto &out = chunkOuts[c];
                            const size_t end = std::min(m2Count, (c + 1) * chunkSize);
                            for (size_t i = c * chunkSize; i < end; ++i) {
                                auto *m2Object = m2Factory->getObjectById<0>(m2ToDraw[i]);
                                if (m2Object == nullptr) continue;
                                collectM2Meshes(m2Object, toIndex(m2ToDraw[i]),
                                                out.opaque, out.transp, out.headers);
                            }
                        }
                    }, tbb::static_partitioner());
                });
            }

            {
                ZoneScopedN("M2 merge");
                size_t totalOpaque = 0, totalTransp = 0, totalHeaders = 0;
                for (auto &out : chunkOuts) {
                    totalOpaque += out.opaque.size();
                    totalTransp += out.transp.size();
                    totalHeaders += out.headers.size();
                }
                frameData.opaqueDraws.reserve(totalOpaque);
                m_transpScratch.reserve(totalTransp);
                m_m2GroupHeaders.reserve(totalHeaders);

                for (auto &out : chunkOuts) {
                    const uint32_t transpBase = m_transpScratch.size();
                    m_transpScratch.insert(m_transpScratch.end(),
                                           std::make_move_iterator(out.transp.begin()),
                                           std::make_move_iterator(out.transp.end()));
                    for (auto &h : out.headers) {
                        h.transpBegin += transpBase;
                        h.transpEnd += transpBase;
                        m_m2GroupHeaders.push_back(h);
                    }
                    frameData.opaqueDraws.insert(frameData.opaqueDraws.end(),
                                                 std::make_move_iterator(out.opaque.begin()),
                                                 std::make_move_iterator(out.opaque.end()));
                }
            }
        } else {
            for (uint32_t i = 0; i < m2Count; i++) {
                auto *m2Object = m2Factory->getObjectById<0>(m2ToDraw[i]);
                if (m2Object == nullptr) continue;
                collectM2Meshes(m2Object, toIndex(m2ToDraw[i]),
                                frameData.opaqueDraws, m_transpScratch, m_m2GroupHeaders);
            }
        }

        // Far-to-near group order for correct transparency
        {
            ZoneScopedN("M2 group sort");
            std::sort(m_m2GroupHeaders.begin(), m_m2GroupHeaders.end(), [](const M2GroupHeader &a, const M2GroupHeader &b) {
                return a.sortDistance > b.sortDistance;
            });
        }

        {
            ZoneScopedN("M2 transp assembly");
            frameData.transparentDraws.reserve(m_transpScratch.size());
            for (auto &header : m_m2GroupHeaders) {
                for (uint32_t j = header.transpBegin; j < header.transpEnd; j++) {
                    frameData.transparentDraws.push_back(m_transpScratch[j]);
                }
            }
        }
    }

    if (m_config->renderWMO) {
        ZoneScopedN("WMO collect");
        for (auto &[group, slot] : m_drawnWmoGroups) {
            uint32_t cullObjectId = m_m2CullCapacity + slot;
            group->forEachGroupMesh([&](const HGMesh &mesh, bool isTransparent) {
                auto *vlkMesh = static_cast<GMeshVLK*>(mesh.get());
                if (isTransparent) {
                    auto &e = frameData.transparentDraws.emplace_back();
                    fillDrawEntry(vlkMesh, cullObjectId, e);
                } else {
                    auto &e = frameData.opaqueDraws.emplace_back();
                    fillDrawEntry(vlkMesh, cullObjectId, e);
                }
            });
        }
    }

    // Sort opaque draws for batching (fewer pipeline/VAO switches)
    {
        ZoneScopedN("opaque sort");
        sortOpaqueDraws(frameData.opaqueDraws);
    }

    // Transparent draws are already ordered by distance (M2 groups far-to-near by
    // bounding-box distance, meshes within each M2 pre-sorted by M2Object, WMO groups
    // after) — don't re-sort
}

void GpuIndirectDrawsVLK::sortOpaqueDraws(std::vector<IndirectDrawEntry> &draws) {
    // Sort opaque draws by (VAO, material) for batching (fewer pipeline/VAO switches)
    std::sort(draws.begin(), draws.end(), [](const IndirectDrawEntry &a, const IndirectDrawEntry &b) {
        if (a.vertexBindingsShared != b.vertexBindingsShared) return a.vertexBindingsShared < b.vertexBindingsShared;
        return a.material < b.material;
    });
}

void GpuIndirectDrawsVLK::uploadActiveList(const FrameDrawData &frameData) {
    ZoneScoped;

    size_t totalDraws = frameData.opaqueDraws.size() + frameData.transparentDraws.size();
    if (totalDraws == 0) return;

    if (totalDraws > m_maxDrawCommands) {
        growDrawCommandStorage((uint32_t)totalDraws);
    }

    m_activeListScratch.resize(totalDraws);
    uint32_t i = 0;
    auto append = [&](const IndirectDrawEntry &e) {
        auto &dst = m_activeListScratch[i++];
        dst.meshId = e.meshId;
        dst.cullObjectId = e.cullObjectId;
        dst.dynFirstIndex = e.dynFirstIndex;
        dst.dynIndexCount = e.dynIndexCount;
    };
    // Layout: [opaque draws][transparent draws] — the liquid passes re-gather only the
    // transparent tail, using the opaque count as the buffer base index.
    for (auto &e : frameData.opaqueDraws) {
        append(e);
    }
    for (auto &e : frameData.transparentDraws) {
        append(e);
    }

    m_activeListBuffer->uploadData(m_activeListScratch.data(), (int)(totalDraws * sizeof(GPUActiveDrawEntry)));
}

// ----------------------
// Draw-stage recording
// ----------------------

void GpuIndirectDrawsVLK::recordFrustumCull(CmdBufRecorder &cmdBuf, const mathfu::mat4 &viewProj, const FrameDrawData &frameData) {
    dispatchFrustumCulling(cmdBuf, viewProj, frameData.totalCullCount);
    dispatchIndirectCulling(cmdBuf, frameData.opaqueDraws.size() + frameData.transparentDraws.size());
}

void GpuIndirectDrawsVLK::recordOcclusionRecull(CmdBufRecorder &cmdBuf, const FrameDrawData &frameData) {
    dispatchIndirectCulling(cmdBuf, frameData.opaqueDraws.size() + frameData.transparentDraws.size());
}

void GpuIndirectDrawsVLK::recordLiquidClassifyAndRecullBelow(CmdBufRecorder &cmdBuf, const mathfu::mat4 &viewProj,
                                                             bool isInvertZ, LiquidHiZVLK *liquidHiZ,
                                                             const FrameDrawData &frameData) {
    dispatchLiquidClassification(cmdBuf, viewProj, frameData.totalCullCount, isInvertZ, liquidHiZ);

    // Re-gather only the transparent tail for below-liquid (skip opaques at the start)
    dispatchIndirectCulling(cmdBuf, frameData.transparentDraws.size(), 0 /*below*/, frameData.opaqueDraws.size());
}

void GpuIndirectDrawsVLK::recordRestoreAndRecullAbove(CmdBufRecorder &cmdBuf, const FrameDrawData &frameData) {
    // Re-gather only the transparent tail for above-liquid. The gather regenerates the
    // commands from the static templates, so no backup/restore of the buffer is needed.
    dispatchIndirectCulling(cmdBuf, frameData.transparentDraws.size(), 1 /*above*/, frameData.opaqueDraws.size());
}

void GpuIndirectDrawsVLK::drawOpaque(CmdBufRecorder &cmdBuf, CmdBufRecorder::ViewportType viewportType, const FrameDrawData &frameData) {
    recordDrawCommands(cmdBuf, frameData.opaqueDraws, 0, viewportType);
}

void GpuIndirectDrawsVLK::drawTransparent(CmdBufRecorder &cmdBuf, CmdBufRecorder::ViewportType viewportType, const FrameDrawData &frameData) {
    recordDrawCommands(cmdBuf, frameData.transparentDraws, frameData.opaqueDraws.size(), viewportType);
}

void GpuIndirectDrawsVLK::recordIndirectRun(CmdBufRecorder &cmdBuf, uint32_t bufferBaseIndex, uint32_t runStart, uint32_t runEnd) {
    if (runEnd == runStart) return;
    if (m_multiDrawIndirectSupported) {
        VkDeviceSize offset = (bufferBaseIndex + runStart) * sizeof(VkDrawIndexedIndirectCommand);
        cmdBuf.drawIndexedIndirect(m_indirectDrawBuffer, offset, runEnd - runStart,
                                   sizeof(VkDrawIndexedIndirectCommand));
    } else {
        for (uint32_t i = runStart; i < runEnd; i++) {
            VkDeviceSize offset = (bufferBaseIndex + i) * sizeof(VkDrawIndexedIndirectCommand);
            cmdBuf.drawIndexedIndirect(m_indirectDrawBuffer, offset, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
        }
    }
}

void GpuIndirectDrawsVLK::recordDrawCommands(CmdBufRecorder &cmdBuf,
                                             const std::vector<IndirectDrawEntry> &draws,
                                             uint32_t bufferBaseIndex,
                                             CmdBufRecorder::ViewportType viewportType) {
    if (draws.empty()) return;
    ZoneScoped;

    cmdBuf.setViewPort(viewportType);
    cmdBuf.setDefaultScissors();

#ifdef DEBUG_MESH_NAMES
    // Per-draw submission with per-draw renderdoc labels (debug only — no batching)
    for (uint32_t i = 0; i < draws.size(); i++) {
        auto const &draw = draws[i];

        auto debugLabel = cmdBuf.beginDebugLabel(draw.debugName, {1.0, 0, 0, 1.0});
        cmdBuf.bindVertexBindings(draw.vertexBindingsShared);
        cmdBuf.bindMaterial(draw.material->shared_from_this());

        VkDeviceSize offset = (bufferBaseIndex + i) * sizeof(VkDrawIndexedIndirectCommand);
        cmdBuf.drawIndexedIndirect(m_indirectDrawBuffer, offset, 1,
                                   sizeof(VkDrawIndexedIndirectCommand));
    }
#else
    // Submit maximal runs of equal (VAO, material): one bind + one multi-draw call
    // per run. The indirect commands are contiguous in the buffer and each is fully
    // self-contained, so a run drawn with drawCount=N executes exactly the same
    // commands, in the same order, as N per-draw calls (culled instanceCount=0
    // entries included). The opaque list is pre-sorted by (VAO, material), so runs
    // are long there; on the depth-sorted transparent list runs form between
    // consecutive same-state entries, which costs nothing when they don't.
    const IVertexBufferBindings *runBindings = nullptr;
    const ISimpleMaterialVLK *runMaterial = nullptr;
    uint32_t runStart = 0;

    auto flushRun = [&](uint32_t runEnd) {
        recordIndirectRun(cmdBuf, bufferBaseIndex, runStart, runEnd);
    };

    for (uint32_t i = 0; i < draws.size(); i++) {
        auto const &draw = draws[i];
        const auto &bindings = draw.vertexBindingsShared.get();
        const auto *material = draw.material;

        if (material != runMaterial || bindings != runBindings) {
            flushRun(i);
            if (bindings != runBindings) {
                cmdBuf.bindVertexBindings(draw.vertexBindingsShared);
                runBindings = bindings;
            }
            if (material != runMaterial) {
                cmdBuf.bindMaterial(draw.material->shared_from_this());
                runMaterial = material;
            }
            runStart = i;
        }
    }
    flushRun(draws.size());
#endif
}

// ----------------------
// Cull dispatches
// ----------------------

void GpuIndirectDrawsVLK::dispatchFrustumCulling(CmdBufRecorder &cmdBuf, const mathfu::mat4 &viewProj, uint32_t objectCount) {
    if (!m_cullComputeMaterial || objectCount == 0) return;

    VkZone(cmdBuf, "Frustum Dispatch");
    auto debugHelper = cmdBuf.beginDebugLabel("Frustum Dispatch", {0, 0, 0.5f});

    // Bind compute pipeline and descriptor sets
    cmdBuf.bindComputePipeline(m_cullComputeMaterial->pipeline);
    cmdBuf.bindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, m_cullComputeMaterial->descriptorSets);

    // Push constants: viewProj matrix + object count
    GPUCullPushConstants pushConstants;
    pushConstants.viewProj = viewProj;
    pushConstants.objectCount = objectCount;

    cmdBuf.pushConstants(
        m_cullComputeMaterial->pipeline->getLayout()->getLayout(),
        VK_SHADER_STAGE_COMPUTE_BIT,
        0,
        sizeof(GPUCullPushConstants),
        &pushConstants
    );

    // Dispatch: 64 threads per workgroup
    uint32_t groupCount = (objectCount + 63) / 64;
    cmdBuf.dispatch(groupCount, 1, 1);

    // Barrier: frustum cull compute write → indirect cull compute read
    std::vector<VkBufferMemoryBarrier> barriers = {{
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_SHADER_WRITE_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        m_visibilityBuffer->getGPUBuffer(),
        0,
        objectCount * sizeof(uint32_t)
    }};

    cmdBuf.recordPipelineBufferBarrier(
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        barriers
    );
}

void GpuIndirectDrawsVLK::dispatchIndirectCulling(CmdBufRecorder &cmdBuf, uint32_t drawCommandCount, uint32_t renderMode, uint32_t startIndex) {
    if (!m_cullIndirectMaterial || drawCommandCount == 0) return;

    cmdBuf.bindComputePipeline(m_cullIndirectMaterial->pipeline);
    cmdBuf.bindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, m_cullIndirectMaterial->descriptorSets);

    mathfu::vec4i pushData(drawCommandCount, renderMode, startIndex, 0);
    cmdBuf.pushConstants(
        m_cullIndirectMaterial->pipeline->getLayout()->getLayout(),
        VK_SHADER_STAGE_COMPUTE_BIT,
        0, sizeof(mathfu::vec4i), &pushData
    );

    uint32_t groupCount = (drawCommandCount + 63) / 64;
    cmdBuf.dispatch(groupCount, 1, 1);

    // Barrier: gather compute write → indirect draw read
    std::vector<VkBufferMemoryBarrier> barriers = {{
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_SHADER_WRITE_BIT,
        VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        m_indirectDrawBuffer->getGPUBuffer(),
        startIndex * sizeof(VkDrawIndexedIndirectCommand),
        drawCommandCount * sizeof(VkDrawIndexedIndirectCommand)
    }};

    cmdBuf.recordPipelineBufferBarrier(
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
        barriers
    );
}

void GpuIndirectDrawsVLK::dispatchLiquidClassification(CmdBufRecorder &cmdBuf,
                                                       const mathfu::mat4 &viewProj,
                                                       uint32_t objectCount, bool isInvertZ,
                                                       LiquidHiZVLK *liquidHiZ) {
    if (!m_classifyComputeMaterial || objectCount == 0) return;
    if (!liquidHiZ || !liquidHiZ->isReady()) return;

    // Create/update descriptor set with aabbBuffer, classificationBuffer, liquidHiZ texture
    if (m_classifyComputeMaterial->descriptorSets.empty()) {
        auto ds = std::make_shared<GDescriptorSet>(m_device, m_classifyComputeMaterial->shader->getDescriptorLayout(0));
        ds->beginUpdate()
            .ssbo(0, m_aabbBuffer)
            .ssbo(1, m_classificationBuffer)
            .texture(2, liquidHiZ->getHiZTexture());
        m_classifyComputeMaterial->descriptorSets.push_back(ds);
    } else {
        m_classifyComputeMaterial->descriptorSets[0]->beginUpdate()
            .ssbo(0, m_aabbBuffer)
            .ssbo(1, m_classificationBuffer)
            .texture(2, liquidHiZ->getHiZTexture());
    }

    cmdBuf.bindComputePipeline(m_classifyComputeMaterial->pipeline);
    cmdBuf.bindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, m_classifyComputeMaterial->descriptorSets);

    bool invertZ = isInvertZ;
    const float vpMinDepth = invertZ ? 0.06f : 0.0f;
    const float vpMaxDepth = invertZ ? 1.0f  : 0.990f;

    struct LiquidClassifyPushConstants {
        mathfu::mat4 viewProj;
        uint32_t objectCount;
        uint32_t hiZMipCount;
        uint32_t hiZWidth;
        uint32_t hiZHeight;
        float viewportDepthMin;
        float viewportDepthRange;
        float pad0;
        float pad1;
    };

    LiquidClassifyPushConstants pc;
    pc.viewProj = viewProj;
    pc.objectCount = objectCount;
    pc.hiZMipCount = liquidHiZ->getMipCount();
    pc.hiZWidth = liquidHiZ->getWidth();
    pc.hiZHeight = liquidHiZ->getHeight();
    pc.viewportDepthMin = vpMinDepth;
    pc.viewportDepthRange = vpMaxDepth - vpMinDepth;
    pc.pad0 = 0;
    pc.pad1 = 0;

    cmdBuf.pushConstants(
        m_classifyComputeMaterial->pipeline->getLayout()->getLayout(),
        VK_SHADER_STAGE_COMPUTE_BIT,
        0, sizeof(LiquidClassifyPushConstants), &pc
    );

    uint32_t groupCount = (objectCount + 63) / 64;
    cmdBuf.dispatch(groupCount, 1, 1);

    // Barrier: classification write → indirect cull read
    std::vector<VkBufferMemoryBarrier> barriers = {{
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_SHADER_WRITE_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        m_classificationBuffer->getGPUBuffer(),
        0,
        objectCount * sizeof(uint32_t)
    }};

    cmdBuf.recordPipelineBufferBarrier(
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        barriers
    );
}
