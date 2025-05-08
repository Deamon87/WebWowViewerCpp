//
// GPU-indirect draw path variant for the bindless renderer.
//
// Bindless meshes of one type share a single VAO and a small set of shared (static)
// materials, so draw entries are grouped into runs of equal (VAO, matId) and each run
// is submitted with one VAO bind + one material bind + one multi-draw-indirect call.
// Per-mesh data is addressed in shaders via firstInstance, exactly as with the CPU-side
// per-command drawIndexed calls this replaces.
//

#ifndef WOWSTUDIO_BINDLESSINDIRECTDRAWSVLK_H
#define WOWSTUDIO_BINDLESSINDIRECTDRAWSVLK_H

#include "GpuIndirectDrawsVLK.h"

class BindlessIndirectDrawsVLK : public GpuIndirectDrawsVLK {
public:
    // Resolves a shared material id to the static material to bind (renderer-owned caches),
    // mirroring the original bindless collector's matId mechanism. Returns nullptr if the
    // id is not registered (the draw run is then skipped, like the old "material checks").
    using MaterialResolver = std::function<std::shared_ptr<ISimpleMaterialVLK>(uint32_t matId)>;

    BindlessIndirectDrawsVLK(const HGDeviceVLK &device, Config *config,
                             MaterialResolver materialResolver);

protected:
    void sortOpaqueDraws(std::vector<IndirectDrawEntry> &draws) override;
    VkDrawIndexedIndirectCommand buildStaticCommand(GMeshVLK *mesh) override;
    void recordDrawCommands(CmdBufRecorder &cmdBuf,
                            const std::vector<IndirectDrawEntry> &draws,
                            uint32_t bufferBaseIndex,
                            CmdBufRecorder::ViewportType viewportType) override;

private:
    MaterialResolver m_materialResolver;
};

#endif //WOWSTUDIO_BINDLESSINDIRECTDRAWSVLK_H
