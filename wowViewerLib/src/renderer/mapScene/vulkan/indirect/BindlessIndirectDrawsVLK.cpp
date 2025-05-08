#include "BindlessIndirectDrawsVLK.h"

#include "../../../frame/FrameProfile.h"
#include "../../../../gapi/vulkan/meshes/GMeshVLK.h"
#include "../../../../gapi/vulkan/materials/ISimpleMaterialVLK.h"

BindlessIndirectDrawsVLK::BindlessIndirectDrawsVLK(const HGDeviceVLK &device, Config *config,
                                                   MaterialResolver materialResolver)
    : GpuIndirectDrawsVLK(device, config),
      m_materialResolver(std::move(materialResolver)) {}

void BindlessIndirectDrawsVLK::sortOpaqueDraws(std::vector<IndirectDrawEntry> &draws) {
    // Group by (VAO, matId) so each run can be submitted with one bind + one multi-draw
    // call; keep priorityPlane ordering within the same material, matching the CPU
    // collector's m2DrawVec sort. matId==0 (materials without a shared id: waterfall,
    // particle, ribbon) falls back to the material pointer as the grouping key.
    std::sort(draws.begin(), draws.end(), [](const IndirectDrawEntry &a, const IndirectDrawEntry &b) {
        if (a.vertexBindingsShared != b.vertexBindingsShared) return a.vertexBindingsShared < b.vertexBindingsShared;
        if (a.matId != b.matId) return a.matId < b.matId;
        if (a.material != b.material) return a.material < b.material;
        return a.priorityPlane < b.priorityPlane;
    });
}

VkDrawIndexedIndirectCommand BindlessIndirectDrawsVLK::buildStaticCommand(GMeshVLK *mesh) {
    auto cmd = GpuIndirectDrawsVLK::buildStaticCommand(mesh);

    // Bindless meshes live in shared mega-buffers and are addressed via firstInstance;
    // meshes without per-instance data need no vertex offset
    // (mirrors COpaqueMeshCollectorBindlessVLK::fillDrawCommand / CmdBufRecorder::drawMesh).
    if (mesh->instanceIndex == -1) {
        cmd.vertexOffset = 0;
    }
    return cmd;
}

void BindlessIndirectDrawsVLK::recordDrawCommands(CmdBufRecorder &cmdBuf,
                                                  const std::vector<IndirectDrawEntry> &draws,
                                                  uint32_t bufferBaseIndex,
                                                  CmdBufRecorder::ViewportType viewportType) {
    if (draws.empty()) return;
    ZoneScoped;

    cmdBuf.setViewPort(viewportType);
    cmdBuf.setDefaultScissors();

    // Submit maximal runs of equal (VAO, matId): one bind + one multi-draw call per run.
    // For the (unsorted) transparent list this preserves the far-to-near order — runs
    // only form between consecutive entries that share the key.
    auto sameRun = [](const IndirectDrawEntry &a, const IndirectDrawEntry &b) {
        if (a.vertexBindingsShared != b.vertexBindingsShared) return false;
        if (a.matId != b.matId) return false;
        // Materials without a shared id can't be merged: they differ per mesh
        if (a.matId == 0 && a.material != b.material) return false;
        return true;
    };

    uint32_t runStart = 0;
    for (uint32_t i = 1; i <= draws.size(); i++) {
        bool runEnds = (i == draws.size()) || !sameRun(draws[i], draws[runStart]);
        if (!runEnds) continue;

        // Resolve the static material by matId (the original bindless "material access
        // and checks"): unresolvable ids skip the run. Materials without a shared id
        // (matId==0: waterfall/particle/ribbon) bind their own per-mesh material.
        std::shared_ptr<ISimpleMaterialVLK> material;
        if (draws[runStart].matId != 0) {
            material = m_materialResolver(draws[runStart].matId);
        } else {
            material = draws[runStart].material->shared_from_this();
        }

        if (material) {
            cmdBuf.bindVertexBindings(draws[runStart].vertexBindingsShared);
            cmdBuf.bindMaterial(material);

            recordIndirectRun(cmdBuf, bufferBaseIndex, runStart, i);
        }

        runStart = i;
    }
}
