//
// Created by Deamon on 22.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IMAPSCENEBUFFERCREATE_H
#define AWEBWOWVIEWERCPP_IMAPSCENEBUFFERCREATE_H

#include <memory>
#include <functional>
#include "../../gapi/interface/IDevice.h"
#include "../../engine/persistance/header/commonFileStructs.h"
#include "materials/IMaterialStructs.h"


PACK(
    struct WMOVertex {
        C3Vector pos;
        C3Vector normal;
        C2Vector textCoordinate;
        C2Vector textCoordinate2;
        C2Vector textCoordinate3;
        C2Vector textCoordinate4;
        CImVector color;
        CImVector color2;
        CImVector colorSecond;
    }
);

PACK(
    struct AdtVertex {
        float height;
        uint8_t normal[4];
        uint8_t mccv[4];
        uint8_t mclv[4];
    }
);

PACK(
    struct LiquidVertexFormat {
        mathfu::vec4_packed pos_transp;
        mathfu::vec2_packed uv;
    }
);

namespace ObjStencilValues {
    constexpr uint8_t ADT_STENCIL_VAL = 1;
    constexpr uint8_t WMO_STENCIL_VAL = 2;
    constexpr uint8_t M2_STENCIL_VAL = 3;
};


static const size_t MAX_PARTICLES_PER_EMITTER = 2000;

//    static const int PARTICLES_BUFF_NUM = IDevice::MAX_FRAMES_IN_FLIGHT + 1;
static const int PARTICLES_BUFF_NUM = IDevice::MAX_FRAMES_IN_FLIGHT;

class IMapSceneBufferCreate {
public:
    virtual ~IMapSceneBufferCreate() = default;

//-------------------------------------
//  Buffer creation
//-------------------------------------

    virtual HGVertexBufferBindings createADTVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) = 0;
    virtual HGVertexBufferBindings createWmoVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) = 0;
    virtual HGVertexBufferBindings createM2VAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) = 0;
    virtual HGVertexBufferBindings createM2ParticleVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) = 0;
    // Pull-model particle VAO for the GPU particle path: index buffer only, no vertex
    // buffers (the vertex shader pulls particle data from SSBOs). Null when unsupported.
    virtual HGVertexBufferBindings createM2ParticleGpuVAO(HGIndexBuffer indexBuffer) { return nullptr; }
    virtual HGVertexBufferBindings createM2RibbonVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) = 0;
    // Pull-model ribbon VAO for the GPU ribbon path: index buffer only, no vertex
    // buffers (the vertex shader reads the ribbon edge SSBO). Null when unsupported.
    virtual HGVertexBufferBindings createM2RibbonGpuVAO(HGIndexBuffer indexBuffer) { return nullptr; }
    // Per-ribbon GPU index buffer chunk in the renderer's GPU-writable index pool
    // (the ribbon sim rewrites its content per frame). Null when unsupported.
    virtual std::shared_ptr<IBuffer> createM2RibbonGpuIndexBuffer(int32_t edgeCount) { return nullptr; }
    virtual HGVertexBufferBindings createWaterVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) = 0;
    virtual HGVertexBufferBindings createSkyVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) = 0;
    virtual HGVertexBufferBindings createPortalVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) = 0;

    virtual HGVertexBuffer createPortalVertexBuffer(int sizeInBytes) = 0;
    virtual HGIndexBuffer  createPortalIndexBuffer(int sizeInBytes) = 0;

    virtual HGVertexBuffer createM2VertexBuffer(int sizeInBytes) = 0;

    virtual HGIndexBuffer  createM2IndexBuffer(int sizeInBytes) = 0;

    virtual HGIndexBuffer  getOrCreateM2ParticleIndexBuffer() = 0;

    virtual HGVertexBuffer createM2ParticleVertexBuffer(int sizeInBytes, int frameIndex) = 0;
    virtual HGVertexBuffer createM2RibbonVertexBuffer(int sizeInBytes) = 0;

    virtual HGVertexBuffer createADTVertexBuffer(int sizeInBytes) = 0;
    virtual HGIndexBuffer  createADTIndexBuffer(int sizeInBytes) = 0;

    virtual HGVertexBuffer createWMOVertexBuffer(int sizeInBytes) = 0;
    virtual HGIndexBuffer  createWMOIndexBuffer(int sizeInBytes) = 0;

    virtual HGVertexBuffer createWaterVertexBuffer(int sizeInBytes) = 0;
    virtual HGIndexBuffer  createWaterIndexBuffer(int sizeInBytes) = 0;

    virtual HGVertexBuffer createSkyVertexBuffer(int sizeInBytes) = 0;
    virtual HGIndexBuffer  createSkyIndexBuffer(int sizeInBytes) = 0;

//-------------------------------------
//  Material creation
//-------------------------------------

    virtual std::shared_ptr<IADTMaterial> createAdtMaterial(const PipelineTemplate &pipelineTemplate,
                                                            const ADTMaterialTemplate &adtMaterialTemplate) = 0;

    virtual std::shared_ptr<IM2ModelData> createM2ModelMat(int bonesCount, int m2ColorsCount, int textureWeightsCount, int textureMatricesCount, uint32_t objectId = 0) = 0;

    // GPU M2 animation path (bindless Vulkan renderer only).
    // supportsM2GpuAnimation() reports availability; createM2GpuAnimData allocates
    // the per-object animation-state slot and sim data. The static track data is
    // shared per source model: modelKey identifies it (the M2Geom pointer — a live
    // track set is always referenced by a live M2Object which keeps its geometry
    // alive, so a live key can never be reused by a different geometry) and
    // packBuilder is invoked at most once per model to produce the packed track
    // data (only when no shared track set exists yet).
    // emitterSeeds/emitterRandomizedTextureIndexMasks carry the CPU emitters' RNG
    // state so the GPU sim continues the same streams (empty when no emitters).
    // Default: unsupported (other backends keep the CPU path).
    virtual bool supportsM2GpuAnimation() const { return false; }
    virtual std::shared_ptr<IM2GpuAnimData> createM2GpuAnimData(
        const void *modelKey,
        const std::function<M2GpuTrackPack()> &packBuilder,
        const std::shared_ptr<IM2ModelData> &m2ModelData,
        const std::vector<M2GpuEmitterSeeds> &emitterSeeds,
        const std::vector<int32_t> &emitterRandomizedTextureIndexMasks,
        const std::vector<std::shared_ptr<IBuffer>> &ribbonGpuIndexBuffers) { return nullptr; }

    virtual std::shared_ptr<IM2Material> createM2Material(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                                          const PipelineTemplate &pipelineTemplate,
                                                          const M2MaterialTemplate &m2MaterialTemplate) = 0;

    virtual std::shared_ptr<IM2ProjectiveMaterial> createM2ProjectiveMaterial(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                                          const PipelineTemplate &pipelineTemplate,
                                                          const M2MaterialTemplate &m2MaterialTemplate) = 0;

    virtual std::shared_ptr<IM2WaterFallMaterial> createM2WaterfallMaterial(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                                          const PipelineTemplate &pipelineTemplate,
                                                          const M2WaterfallMaterialTemplate &m2MaterialTemplate) = 0;

    virtual std::shared_ptr<IM2ParticleMaterial> createM2ParticleMaterial(const PipelineTemplate &pipelineTemplate,
                                                                          const M2ParticleMaterialTemplate &m2MaterialTemplate) = 0;

    virtual std::shared_ptr<IM2RibbonMaterial> createM2RibbonMaterial(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                                                      const PipelineTemplate &pipelineTemplate,
                                                                      const M2RibbonMaterialTemplate &m2RibbonMaterialTemplate) = 0;

    virtual std::shared_ptr<ISkyMeshMaterial> createSkyMeshMaterial(const PipelineTemplate &pipelineTemplate) = 0;

    virtual std::shared_ptr<IPlanetMaterial> createPlanetMaterial(const PipelineTemplate &pipelineTemplate,
                                                                  const HGSamplableTexture &texture) = 0;

    virtual std::shared_ptr<IWmoModelData> createWMOWideChunk(int groupNum) = 0;
    virtual std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> createWmoModelMatrixChunk() = 0;

    virtual std::shared_ptr<IWMOMaterial> createWMOMaterial(const std::shared_ptr<IWmoModelData> &wmoModelWide,
                                                            const PipelineTemplate &pipelineTemplate,
                                                            const WMOMaterialTemplate &wmoMaterialTemplate) = 0;
    virtual std::shared_ptr<IPortalMaterial> createPortalMaterial(const PipelineTemplate &pipelineTemplate) = 0;

//-------------------------------------
// Liquid material creation
//-------------------------------------
    virtual std::shared_ptr<ILiquidMaterial> createLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
                                                               const PipelineTemplate &pipelineTemplate,
                                                               const WaterMaterialTemplate &waterMaterialTemplate) = 0;

    virtual std::shared_ptr<ILiquidMaterial> createWaterLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<WaterLiquidData> &liquidData) = 0;
    virtual std::shared_ptr<ILiquidMaterial> createMagmaLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<MagmaLiquidData> &liquidData) = 0;
    virtual std::shared_ptr<ILiquidMaterial> createMercuryLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<MercuryLiquidData> &liquidData) = 0;
    virtual std::shared_ptr<ILiquidMaterial> createFogLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<FogLiquidData> &liquidData) = 0;
    virtual std::shared_ptr<ILiquidMaterial> createLeyLineLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<LeyLineLiquidData> &liquidData) = 0;
    virtual std::shared_ptr<ILiquidMaterial> createFelLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<FelLiquidData> &liquidData) = 0;
    virtual std::shared_ptr<ILiquidMaterial> createSwampLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<SwampLiquidData> &liquidData) = 0;
    virtual std::shared_ptr<ILiquidMaterial> createAzeritheLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<AzeritheLiquidData> &liquidData) = 0;

    virtual std::shared_ptr<WaterLiquidData> createWaterLiquidData() = 0;
    virtual std::shared_ptr<MagmaLiquidData> createMagmaLiquidData() = 0;
    virtual std::shared_ptr<MercuryLiquidData> createMercuryLiquidData() = 0;
    virtual std::shared_ptr<FogLiquidData> createFogLiquidData() = 0;
    virtual std::shared_ptr<LeyLineLiquidData> createLeyLineLiquidData() = 0;
    virtual std::shared_ptr<FelLiquidData> createFelLiquidData() = 0;
    virtual std::shared_ptr<SwampLiquidData> createSwampLiquidData() = 0;
    virtual std::shared_ptr<AzeritheLiquidData> createAzeritheLiquidData() = 0;

//-------------------------------------
//  Mesh creation
//-------------------------------------

    virtual HGMesh createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) = 0;
    virtual HGSortableMesh createSortableMesh(gMeshTemplate &meshTemplate, const HMaterial &material, int priorityPlane) = 0;
    virtual HGMesh createAdtMesh(gMeshTemplate &meshTemplate,  const std::shared_ptr<IADTMaterial> &material) = 0;
    virtual HGM2Mesh createM2Mesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2Material> &material, int layer, int priorityPlane) = 0;
    virtual HGM2Mesh createM2ProjectiveMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2ProjectiveMaterial> &material, int layer, int priorityPlane) = 0;
    virtual HGM2Mesh createM2ParticleMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2Material> &material, int layer, int priorityPlane) = 0;
    virtual HGSortableMesh createWaterMesh(gMeshTemplate &meshTemplate, const HMaterial &material, int priorityPlane) = 0;
    virtual HGSortableMesh createWMOMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IWMOMaterial> &material, int groupNum, int canHaveExteriorLit, uint32_t wmoObjId = 0) = 0;
    virtual HGM2Mesh createM2WaterfallMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2WaterFallMaterial> &material, int layer, int priorityPlane) = 0;
};
typedef std::shared_ptr<IMapSceneBufferCreate> HMapSceneBufferCreate;

#endif //AWEBWOWVIEWERCPP_IMAPSCENEBUFFERCREATE_H
