//
// GPU-indirect draw path for the map scene.
//
// Draw parameters of a mesh are static for its lifetime (particles/ribbons excepted —
// they patch start/end per frame), so each mesh's VkDrawIndexedIndirectCommand template
// is written once (registerMesh) into a static GPU buffer indexed by meshId. Per frame
// the CPU only builds a compact "active draw list" of {meshId, cullObjectId, overrides}
// records in draw order; a compute gather pass then instantiates the final indirect
// commands from the templates, applying frustum/occlusion/liquid visibility through
// cullObjectId.
//
// Cull ids are stable across frames:
//   [0, m_m2CullCapacity)                          — M2 factory ids. The M2 factory keeps a
//                                                    CAaBox component per object; it is
//                                                    bulk-mirrored into the AABB buffer when
//                                                    the factory marks the component dirty.
//   [m_m2CullCapacity, + m_wmoGroupCullSlots)      — WMO-group slots from a persistent
//                                                    registry (group AABBs never change, so
//                                                    they are uploaded once per group).
//
// All capacities are only initial sizes: when the scene outgrows one, the region doubles
// and the underlying HGBufferVLK is resized (the GPU reallocation copies the old content
// and rebinds the descriptor sets through the buffer's on-handle-change callback). When
// the M2 region grows, the WMO region base shifts — group AABBs are re-uploaded from the
// CPU mirror at the new base and cull ids are recomputed per frame, so nothing breaks.
//
// The renderer records draws against the per-frame compact command buffer at offsets
// derived from the active-list position; recordDrawCommands() remains the seam for the
// submission strategy (both variants batch runs of equal VAO/material into multi-draws;
// bindless additionally resolves shared materials by matId).
//

#ifndef WOWSTUDIO_GPUINDIRECTDRAWSVLK_H
#define WOWSTUDIO_GPUINDIRECTDRAWSVLK_H

#include <array>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "../../../../gapi/vulkan/GDeviceVulkan.h"
#include "../../../../gapi/vulkan/materials/ComputeMaterialBuilderVLK.h"
#include "../../../../gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferRecorder.h"
#include "../../../../gapi/UniformBufferStructures.h" // PACK macro
#include "../../../../engine/persistance/header/commonFileStructs.h" // CAaBox
#include "../view/LiquidHiZVLK.h"

class Config;
class GMeshVLK;
class ISimpleMaterialVLK;
class IVertexBufferBindings;
class M2Object;
class WmoGroupObject;
struct MapRenderPlan;

class GpuIndirectDrawsVLK {
public:
    // Cull-id space layout (see file header). These are initial capacities only —
    // each region doubles on demand when the scene outgrows it.
    static constexpr uint32_t INITIAL_M2_CULL_CAPACITY = 131072;
    static constexpr uint32_t INITIAL_WMO_GROUP_CULL_SLOTS = 32768;
    static constexpr uint32_t STATIC_CMD_CAPACITY = 500000;   // initial static-template capacity, indexed by meshId
    static constexpr uint32_t STATIC_CMD_MAX_CAPACITY = 1u << 23; // growth safety cap (8M meshes, 160MB of commands)
    static constexpr uint32_t INITIAL_MAX_DRAW_COMMANDS = 500000; // initial per-frame indirect command capacity

    // One active-list record (the GPU-uploaded part) plus the CPU-side info needed to
    // record the draw (material/VAO binding, run batching in bindless).
    struct IndirectDrawEntry {
        uint32_t meshId = 0;         // index into the static command buffer
        uint32_t cullObjectId = 0;   // M2 factory id, or m_m2CullCapacity + wmo group slot
        uint32_t dynFirstIndex = 0;  // particle/ribbon draw-param override...
        uint32_t dynIndexCount = 0;  // ...applied by the gather pass when != 0
        ISimpleMaterialVLK *material = nullptr;                    // for binding/batching
        uint32_t matId = 0;          // shared material id (bindless run batching)
        uint32_t priorityPlane = 0;
        std::shared_ptr<IVertexBufferBindings> vertexBindingsShared;
#ifdef DEBUG_MESH_NAMES
        std::string debugName;       // copied from the mesh at collection time (renderdoc labels)
#endif
    };

    // Per-frame snapshot of the built draw data. The draw stage may run while the next
    // frame's update is already rebuilding, so the component keeps MAX_FRAMES_IN_FLIGHT
    // slots and hands out one per build; the draw lambda captures the shared_ptr.
    struct FrameDrawData {
        std::vector<IndirectDrawEntry> opaqueDraws;
        std::vector<IndirectDrawEntry> transparentDraws;
        uint32_t totalCullCount = 0;   // cull-id space covered by the cull dispatches
        uint32_t m2AabbCount = 0;      // valid entries in the M2 region of the AABB buffer
        uint32_t wmoGroupBase = 0;     // index of the WMO-group region start in the cull buffers
        uint32_t wmoGroupCount = 0;    // used slots in the WMO-group region (base = wmoGroupBase)
    };

    GpuIndirectDrawsVLK(const HGDeviceVLK &device, Config *config);
    virtual ~GpuIndirectDrawsVLK();

    // Creates the GPU buffers and compute materials and registers the uploadable
    // buffers into the renderer's shared upload list. Called once on renderer creation.
    void setup(std::vector<std::reference_wrapper<HGBufferVLK>> &allBuffers);

    // Writes the mesh's static draw-command template (called once per created mesh by
    // the renderer's create*Mesh functions, after start/end/instanceIndex are final).
    // Doubles the static command storage when meshId exceeds the current capacity.
    void registerMesh(const std::shared_ptr<GMeshVLK> &mesh);

    // Update stage: syncs static/AABB data to the GPU (dirty ranges only), collects the
    // frame's active draw list and queues its upload.
    std::shared_ptr<FrameDrawData> buildFrameDraws(const std::shared_ptr<MapRenderPlan> &framePlan);

    // Draw stage: frustum culling of all objects + gather of the indirect commands.
    void recordFrustumCull(CmdBufRecorder &cmdBuf, const mathfu::mat4 &viewProj, const FrameDrawData &frameData);
    // Re-gather of the indirect commands after Hi-Z occlusion culling updated visibility.
    void recordOcclusionRecull(CmdBufRecorder &cmdBuf, const FrameDrawData &frameData);
    // Liquid classification + below-liquid gather of the transparent tail.
    void recordLiquidClassifyAndRecullBelow(CmdBufRecorder &cmdBuf, const mathfu::mat4 &viewProj,
                                            bool isInvertZ, LiquidHiZVLK *liquidHiZ, const FrameDrawData &frameData);
    // Above-liquid gather of the transparent tail (regenerates from templates — no
    // backup/restore needed, unlike the old in-place culling).
    void recordRestoreAndRecullAbove(CmdBufRecorder &cmdBuf, const FrameDrawData &frameData);

    void drawOpaque(CmdBufRecorder &cmdBuf, CmdBufRecorder::ViewportType viewportType, const FrameDrawData &frameData);
    void drawTransparent(CmdBufRecorder &cmdBuf, CmdBufRecorder::ViewportType viewportType, const FrameDrawData &frameData);

    // Accessors for renderer-side features built on top of the cull data
    // (Hi-Z occlusion culling, AABB debug draw).
    const HGBufferVLK &aabbBuffer() const { return m_aabbBuffer; }
    const HGBufferVLK &visibilityBuffer() const { return m_visibilityBuffer; }

    // Persistent WMO-group cull slots. Also used by the selection-highlight update to
    // find live group objects (update thread only).
    struct WmoGroupCullEntry {
        uint32_t slot = UINT32_MAX;      // index into the WMO region of the cull buffers
        std::weak_ptr<WmoGroupObject> weak; // for sweeping dead groups
        bool aabbWritten = false;        // group AABB uploaded (groups load async)
        uint32_t lastSeenFrame = UINT32_MAX; // per-frame dedup of the drawn-group list
    };
    const std::unordered_map<WmoGroupObject*, WmoGroupCullEntry> &wmoGroupCullIndexMap() const { return m_wmoGroupRegistry; }

protected:
    // Submission strategy seam: the default batches runs of equal (VAO, material)
    // into multi-draw calls (one bind + one drawIndexedIndirect per run). Bindless
    // additionally resolves shared materials by matId before submitting a run.
    virtual void recordDrawCommands(CmdBufRecorder &cmdBuf,
                                    const std::vector<IndirectDrawEntry> &draws,
                                    uint32_t bufferBaseIndex,
                                    CmdBufRecorder::ViewportType viewportType);

    // Sorting seam for the opaque draw list: the default sorts by (VAO, material) for
    // batching. Bindless sorts by (VAO, matId, priorityPlane) to form multi-draw runs.
    virtual void sortOpaqueDraws(std::vector<IndirectDrawEntry> &draws);

    // Static command template written at registration. Bindless overrides the
    // vertex-offset rule (its meshes live in shared mega-buffers).
    virtual VkDrawIndexedIndirectCommand buildStaticCommand(GMeshVLK *mesh);

    const HGBufferVLK &indirectDrawBuffer() const { return m_indirectDrawBuffer; }

    // vkCmdDrawIndexedIndirect with drawCount > 1 needs the multiDrawIndirect feature;
    // when absent, runs are submitted as individual drawCount=1 calls.
    bool m_multiDrawIndirectSupported = false;

    // Submits one batched run of contiguous indirect commands (single draws when
    // multiDrawIndirect is unsupported).
    void recordIndirectRun(CmdBufRecorder &cmdBuf, uint32_t bufferBaseIndex, uint32_t runStart, uint32_t runEnd);

private:
    struct M2GroupHeader {
        float sortDistance = 0;
        uint32_t transpBegin = 0;
        uint32_t transpEnd = 0;
    };

    void fillDrawEntry(GMeshVLK *mesh, uint32_t cullObjectId, IndirectDrawEntry &entry);
    void collectM2Meshes(M2Object *m2Object, uint32_t cullObjectId,
                         std::vector<IndirectDrawEntry> &opaqueDraws,
                         std::vector<IndirectDrawEntry> &transpDraws,
                         std::vector<M2GroupHeader> &groupHeaders);
    void collectMeshDraws(const std::shared_ptr<MapRenderPlan> &framePlan, FrameDrawData &frameData);
    void collectWmoGroups(const std::shared_ptr<MapRenderPlan> &framePlan);
    void syncStaticDrawCommands();
    // Doubles the static command storage (CPU mirror + GPU buffer) until meshId fits.
    // Must be called with m_staticCmdMutex held. Returns false when meshId is beyond
    // STATIC_CMD_MAX_CAPACITY.
    bool growStaticCmdStorage(uint32_t meshId);
    void syncM2Aabbs();
    void uploadActiveList(const FrameDrawData &frameData);

    // Resizes the shared cull buffers (AABB/visibility/classification) to the current
    // total cull capacity. Content is preserved by GBufferVLK's reallocation copy.
    void resizeCullBuffers();
    // Doubles the M2 cull region until requiredCapacity fits. The WMO region base
    // shifts, so the WMO AABB mirror is re-uploaded at the new base.
    void growM2CullRegion(uint32_t requiredCapacity);
    // Doubles the WMO-group cull region (CPU mirror + cull buffers).
    void growWmoGroupSlots();
    // Doubles the per-frame draw storage (indirect + active-list buffers) until
    // requiredCount fits.
    void growDrawCommandStorage(uint32_t requiredCount);

    void dispatchFrustumCulling(CmdBufRecorder &cmdBuf, const mathfu::mat4 &viewProj, uint32_t objectCount);
    void dispatchIndirectCulling(CmdBufRecorder &cmdBuf, uint32_t drawCommandCount, uint32_t renderMode = 2, uint32_t startIndex = 0);
    void dispatchLiquidClassification(CmdBufRecorder &cmdBuf, const mathfu::mat4 &viewProj,
                                      uint32_t objectCount, bool isInvertZ, LiquidHiZVLK *liquidHiZ);

    HGDeviceVLK m_device;
    Config *m_config;

    HGBufferVLK m_aabbBuffer;            // GPUAabb[m_m2CullCapacity + m_wmoGroupCullSlots]
    HGBufferVLK m_visibilityBuffer;      // uint32[m_m2CullCapacity + m_wmoGroupCullSlots]
    HGBufferVLK m_classificationBuffer;  // uint32[m_m2CullCapacity + m_wmoGroupCullSlots]
    HGBufferVLK m_indirectDrawBuffer;    // VkDrawIndexedIndirectCommand[m_maxDrawCommands], GPU-written per frame
    HGBufferVLK m_staticCmdBuffer;       // VkDrawIndexedIndirectCommand[m_staticCmdCapacity], write-once templates
    HGBufferVLK m_activeListBuffer;      // GPUActiveDrawEntry[m_maxDrawCommands], per-frame upload
    std::shared_ptr<ComputeMaterialVLK> m_cullComputeMaterial;
    std::shared_ptr<ComputeMaterialVLK> m_cullIndirectMaterial;
    std::shared_ptr<ComputeMaterialVLK> m_classifyComputeMaterial;

    // Current cull/draw capacities in elements; each doubles on demand (update thread only)
    uint32_t m_m2CullCapacity = INITIAL_M2_CULL_CAPACITY;
    uint32_t m_wmoGroupCullSlots = INITIAL_WMO_GROUP_CULL_SLOTS;
    uint32_t m_maxDrawCommands = INITIAL_MAX_DRAW_COMMANDS;

    // Static command templates: CPU mirror + dirty span, flushed in buildFrameDraws
    std::mutex m_staticCmdMutex;
    std::vector<VkDrawIndexedIndirectCommand> m_staticCmdMirror;
    uint32_t m_staticCmdCapacity = STATIC_CMD_CAPACITY; // current capacity in elements (doubles on demand)
    uint32_t m_staticDirtyMin = UINT32_MAX;
    uint32_t m_staticDirtyMax = 0;

    // WMO-group persistent cull slots
    std::unordered_map<WmoGroupObject*, WmoGroupCullEntry> m_wmoGroupRegistry;
    std::vector<uint32_t> m_wmoGroupFreeSlots;
    uint32_t m_wmoGroupHighWater = 0;
    uint32_t m_frameCounter = 0;

    struct GPUAabb {
        mathfu::vec4_packed min;
        mathfu::vec4_packed max;
    };
    std::vector<GPUAabb> m_wmoAabbMirror;      // CPU mirror of the WMO-group AABB region
    uint32_t m_wmoAabbDirtyMin = UINT32_MAX;
    uint32_t m_wmoAabbDirtyMax = 0;

    // Update-thread scratch, reused across frames (capacity persists)
    std::vector<CAaBox> m_m2AabbScratch;
    std::vector<GPUAabb> m_gpuAabbScratch;
    uint32_t m_syncedM2AabbCount = 0;

    std::vector<IndirectDrawEntry> m_transpScratch;
    std::vector<M2GroupHeader> m_m2GroupHeaders;
    std::vector<std::pair<WmoGroupObject*, uint32_t>> m_drawnWmoGroups;

    struct GPUActiveDrawEntry {
        uint32_t meshId;
        uint32_t cullObjectId;
        uint32_t dynFirstIndex;
        uint32_t dynIndexCount;
    };
    std::vector<GPUActiveDrawEntry> m_activeListScratch;

    std::array<std::shared_ptr<FrameDrawData>, IDevice::MAX_FRAMES_IN_FLIGHT> m_frameSlots;
    uint32_t m_buildCounter = 0;

    PACK(
    struct GPUCullPushConstants {
        mathfu::mat4 viewProj;
        uint32_t objectCount;
        uint32_t pad1;
        uint32_t pad2;
        uint32_t pad3;
    });
};

#endif //WOWSTUDIO_GPUINDIRECTDRAWSVLK_H
