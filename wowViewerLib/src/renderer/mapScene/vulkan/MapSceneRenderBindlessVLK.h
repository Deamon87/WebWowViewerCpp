//
// Created by Deamon on 12/1/2022.
//

#ifndef AWEBWOWVIEWERCPP_MAPSCENERENDERBINDLESSVLK_H
#define AWEBWOWVIEWERCPP_MAPSCENERENDERBINDLESSVLK_H


#include "../MapSceneRenderer.h"
#include "../../../gapi/vulkan/GDeviceVulkan.h"
#include "../../../gapi/vulkan/buffers/GBufferChunkDynamicVersionedVLK.h"
#include "../materials/IMaterialStructs.h"
#include "passes/FFXGlowPassVLK.h"
#include "../../../gapi/vulkan/materials/ISimpleMaterialVLK.h"
#include "view/RenderViewForwardVLK.h"
#include "../../../gapi/vulkan/descriptorSets/bindless/BindlessTextureHolder.h"
#include "view/RenderViewDeferredVLK.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/meshes/GMeshVLK.h"
#include "../../../engine/objects/scenes/EntityActorsFactory.h"
#include "indirect/BindlessIndirectDrawsVLK.h"
#include "../../../gapi/vulkan/buffers/CBufferChunkVLK.h"
#include "GpuM2AnimationVLK.h"

class COpaqueMeshCollectorBindlessVLK;

class MapSceneRenderBindlessVLK : public MapSceneRenderer {
    friend class COpaqueMeshCollectorBindlessVLK;
public:
    explicit MapSceneRenderBindlessVLK(const HGDeviceVLK &hDevice, Config *config);
    ~MapSceneRenderBindlessVLK() override {
        std::cout << "destroy" << std::endl;
    };

    std::unique_ptr<IRenderFunction> update(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams, const std::shared_ptr<MapRenderPlan> &framePlan) override;

    std::shared_ptr<MapRenderPlan> getLastCreatedPlan() override;

//-------------------------------------
//  Buffer creation
//-------------------------------------
    HGVertexBufferBindings createADTVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) override;
    HGVertexBufferBindings createWmoVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) override;
    HGVertexBufferBindings createM2VAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) override;
    HGVertexBufferBindings createM2ParticleVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) override;
    HGVertexBufferBindings createM2ParticleGpuVAO(HGIndexBuffer indexBuffer) override;
    HGVertexBufferBindings createM2RibbonVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) override;
    HGVertexBufferBindings createM2RibbonGpuVAO(HGIndexBuffer indexBuffer) override;
    std::shared_ptr<IBuffer> createM2RibbonGpuIndexBuffer(int32_t edgeCount) override;
    HGVertexBufferBindings createWaterVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) override;
    HGVertexBufferBindings createSkyVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) override;
    HGVertexBufferBindings createPortalVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) override;

    HGVertexBuffer createPortalVertexBuffer(int sizeInBytes) override;
    HGIndexBuffer  createPortalIndexBuffer(int sizeInBytes) override;

    HGVertexBuffer createM2VertexBuffer(int sizeInBytes) override;
    HGIndexBuffer  createM2IndexBuffer(int sizeInBytes) override;

    HGIndexBuffer  getOrCreateM2ParticleIndexBuffer() override;

    HGVertexBuffer createM2ParticleVertexBuffer(int sizeInBytes, int frameIndex) override;
    HGVertexBuffer createM2RibbonVertexBuffer(int sizeInBytes) override;

    HGVertexBuffer createADTVertexBuffer(int sizeInBytes) override;
    HGIndexBuffer  createADTIndexBuffer(int sizeInBytes) override;

    HGVertexBuffer createWMOVertexBuffer(int sizeInBytes) override;
    HGIndexBuffer  createWMOIndexBuffer(int sizeInBytes) override;

    HGVertexBuffer createWaterVertexBuffer(int sizeInBytes) override;
    HGIndexBuffer  createWaterIndexBuffer(int sizeInBytes) override;

    HGVertexBuffer createSkyVertexBuffer(int sizeInBytes) override;
    HGIndexBuffer  createSkyIndexBuffer(int sizeInBytes) override;

//-------------------------------------
//  Material creation
//-------------------------------------

    std::shared_ptr<IADTMaterial> createAdtMaterial(const PipelineTemplate &pipelineTemplate, const ADTMaterialTemplate &adtMaterialTemplate) override;
    std::shared_ptr<IM2ModelData> createM2ModelMat(int bonesCount, int m2ColorsCount, int textureWeightsCount, int textureMatricesCount, uint32_t objectId = 0) override;

    // GPU M2 animation (m2Animation compute pass writes bones into m2Buffers.boneMatrix)
    bool supportsM2GpuAnimation() const override { return m_gpuM2Animation.isEnabled(); }
    std::shared_ptr<IM2GpuAnimData> createM2GpuAnimData(const void *modelKey,
                                                        const std::function<M2GpuTrackPack()> &packBuilder,
                                                        const std::shared_ptr<IM2ModelData> &m2ModelData,
                                                        const std::vector<M2GpuEmitterSeeds> &emitterSeeds,
                                                        const std::vector<int32_t> &emitterRandomizedTextureIndexMasks,
                                                        const std::vector<std::shared_ptr<IBuffer>> &ribbonGpuIndexBuffers) override;
    std::shared_ptr<IM2Material> createM2Material(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                                  const PipelineTemplate &pipelineTemplate,
                                                  const M2MaterialTemplate &m2MaterialTemplate) override;
    std::shared_ptr<IM2ProjectiveMaterial> createM2ProjectiveMaterial(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                                  const PipelineTemplate &pipelineTemplate,
                                                  const M2MaterialTemplate &m2MaterialTemplate) override;
    std::shared_ptr<IM2WaterFallMaterial> createM2WaterfallMaterial(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                                                    const PipelineTemplate &pipelineTemplate,
                                                                    const M2WaterfallMaterialTemplate &m2MaterialTemplate) override;

    std::shared_ptr<IM2ParticleMaterial> createM2ParticleMaterial(const PipelineTemplate &pipelineTemplate,
                                                                  const M2ParticleMaterialTemplate &m2MaterialTemplate) override;
    std::shared_ptr<IM2RibbonMaterial> createM2RibbonMaterial(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                                              const PipelineTemplate &pipelineTemplate,
                                                              const M2RibbonMaterialTemplate &m2RibbonMaterialTemplate) override;

    std::shared_ptr<IWmoModelData> createWMOWideChunk(int groupNum)  override;
    std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> createWmoModelMatrixChunk() override;


    std::shared_ptr<IWMOMaterial> createWMOMaterial(const std::shared_ptr<IWmoModelData> &wmoModelWide,
                                                    const PipelineTemplate &pipelineTemplate,
                                                    const WMOMaterialTemplate &wmoMaterialTemplate) override;
    std::shared_ptr<ILiquidMaterial> createLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
                                                        const PipelineTemplate &pipelineTemplate,
                                                        const WaterMaterialTemplate &waterMaterialTemplate) override;

    std::shared_ptr<ISkyMeshMaterial> createSkyMeshMaterial(const PipelineTemplate &pipelineTemplate) override;

    std::shared_ptr<IPlanetMaterial> createPlanetMaterial(const PipelineTemplate &pipelineTemplate,
                                                          const HGSamplableTexture &texture) override;

    std::shared_ptr<IPortalMaterial> createPortalMaterial(const PipelineTemplate &pipelineTemplate) override;

//--------------------------
// Liquid Material creation
//--------------------------

    std::shared_ptr<ILiquidMaterial> createWaterLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<WaterLiquidData> &liquidData) override;
    std::shared_ptr<ILiquidMaterial> createMagmaLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<MagmaLiquidData> &liquidData) override;
    std::shared_ptr<ILiquidMaterial> createMercuryLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<MercuryLiquidData> &liquidData) override;
    std::shared_ptr<ILiquidMaterial> createFogLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<FogLiquidData> &liquidData) override;
    std::shared_ptr<ILiquidMaterial> createLeyLineLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<LeyLineLiquidData> &liquidData) override;
    std::shared_ptr<ILiquidMaterial> createFelLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<FelLiquidData> &liquidData) override;
    std::shared_ptr<ILiquidMaterial> createSwampLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<SwampLiquidData> &liquidData) override;
    std::shared_ptr<ILiquidMaterial> createAzeritheLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<AzeritheLiquidData> &liquidData) override;

//-------------------------------------
//  Mesh creation
//-------------------------------------

    HGMesh createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) override;
    HGSortableMesh createSortableMesh(gMeshTemplate &meshTemplate, const HMaterial &material, int priorityPlane) override;
    HGMesh createAdtMesh(gMeshTemplate &meshTemplate,  const std::shared_ptr<IADTMaterial> &material) override;
    HGM2Mesh createM2Mesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2Material> &material, int layer, int priorityPlane) override;
    HGM2Mesh createM2ProjectiveMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2ProjectiveMaterial> &material, int layer, int priorityPlane) override;
    HGM2Mesh createM2ParticleMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2Material> &material, int layer, int priorityPlane) override;
    HGSortableMesh createWaterMesh(gMeshTemplate &meshTemplate, const HMaterial &material, int priorityPlane) override;
    HGSortableMesh createWMOMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IWMOMaterial> &material, int groupNum, int canHaveExteriorLit, uint32_t wmoObjId = 0) override;
    HGM2Mesh createM2WaterfallMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2WaterFallMaterial> &material, int layer, int priorityPlane) override;

//--------------------------------------
// RenderView
//--------------------------------------

    std::shared_ptr<IRenderView> createRenderView(bool createOutput) override;

    std::shared_ptr<WaterLiquidData> createWaterLiquidData() override;
    std::shared_ptr<MagmaLiquidData> createMagmaLiquidData() override;
    std::shared_ptr<MercuryLiquidData> createMercuryLiquidData() override;
    std::shared_ptr<FogLiquidData> createFogLiquidData() override;
    std::shared_ptr<LeyLineLiquidData> createLeyLineLiquidData() override;
    std::shared_ptr<FelLiquidData> createFelLiquidData() override;
    std::shared_ptr<SwampLiquidData> createSwampLiquidData() override;
    std::shared_ptr<AzeritheLiquidData> createAzeritheLiquidData() override;

protected:
    template<typename T>
    class LiquidDataBindlessVLK : public LiquidData<T> {
    public:
        explicit LiquidDataBindlessVLK(const std::shared_ptr<BindlessTextureHolder> &textureHolder, const std::shared_ptr<IBufferChunk<T>> &liquidData);
        ~LiquidDataBindlessVLK() override = default;

        void updateAnimatedTextures(const std::array<HGSamplableTexture, MAX_LIQUID_TYPE_TEXTURES + 1> &textures, float currentTime) override;

        // Registers a per-instance LiquidBindless record + the global liquid texture array,
        // so animated texture indices can be published for the bindless shader.
        // Multiple liquid materials can share one LiquidData (per-liquidTypeId cache), so
        // targets accumulate — every registered record is rewritten in full on each update
        // (getObject() hands out a fresh staging span per call; partial writes upload garbage).
        void setBindlessUpdateTarget(const std::shared_ptr<IBufferChunk<Liquid::LiquidBindless>> &bindlessChunk,
                                     const std::shared_ptr<GDescriptorSet> &texturesDS,
                                     int fallbackTextureIndex,
                                     int liquidDataInd, int placementMatInd, int liquidMaterialType);

    private:
        struct BindlessTarget {
            std::weak_ptr<IBufferChunk<Liquid::LiquidBindless>> chunk;
            int liquidDataInd = 0;
            int placementMatInd = 0;
            int liquidMaterialType = 0;
        };
        std::shared_ptr<BindlessTextureHolder> m_textureHolder;
        std::array<std::shared_ptr<BindlessTexture>, 9> m_bindlessTextures;
        std::vector<BindlessTarget> m_bindlessTargets;
        std::shared_ptr<GDescriptorSet> m_texturesDS = nullptr;
        int m_fallbackTextureIndex = 0;
    };

    template<typename T>
    std::shared_ptr<ILiquidMaterial> createLiquidMaterialBindless(
        const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
        const std::shared_ptr<LiquidData<T>> &liquidData,
        Liquid::LiquidMaterialType materialType);

    /*virtual*/ std::shared_ptr<ISimpleMaterialVLK> getM2StaticMaterial(const PipelineTemplate &pipelineTemplate);
    /*virtual*/ std::shared_ptr<ISimpleMaterialVLK> getM2ProjectiveStaticMaterial(const PipelineTemplate &pipelineTemplate);
    /*virtual*/ std::shared_ptr<ISimpleMaterialVLK> getWMOStaticMaterial(const PipelineTemplate &pipelineTemplate);

    struct PipelineTemplateHasher {
        std::size_t operator()(const PipelineTemplate& k) const {
            using std::hash;
            return (hash<bool >{}(k.backFaceCulling) << 2) ^
                   (hash<bool >{}(k.triCCW) << 4) ^
                   (hash<bool >{}(k.depthCulling) << 8) ^
                   (hash<bool >{}(k.depthWrite) << 10) ^
                   (hash<EGxBlendEnum>{}(k.blendMode) << 14) ^
                   (hash<DrawElementMode>{}(k.element) << 16) ^
                   (hash<uint8_t>{}(k.colorMask) << 18);
        };
    };
    robin_hood::unordered_flat_map<PipelineTemplate, std::shared_ptr<ISimpleMaterialVLK>, PipelineTemplateHasher> m_m2StaticMaterials;
    robin_hood::unordered_flat_map<PipelineTemplate, std::shared_ptr<ISimpleMaterialVLK>, PipelineTemplateHasher> m_m2ProjectiveStaticMaterials;
    robin_hood::unordered_flat_map<PipelineTemplate, std::shared_ptr<ISimpleMaterialVLK>, PipelineTemplateHasher> m_wmoStaticMaterials;
protected:
    HGDeviceVLK m_device;

    int m_width = 640;
    int m_height = 480;

    HGBufferVLK vboM2Buffer;
    std::array<HGBufferVLK, PARTICLES_BUFF_NUM> vboM2ParticleBuffers;
    HGBufferVLK vboM2RibbonBuffer;
    HGBufferVLK vboPortalBuffer;

    HGBufferVLK vboAdtBuffer;
    HGBufferVLK vboWMOBuffer;
    HGBufferVLK vboWaterBuffer;
    HGBufferVLK vboSkyBuffer;

    HGBufferVLK iboBuffer;

    HGBufferVLK uboStaticBuffer;
    HGBufferVLK pointLightBuffer;
    HGBufferVLK spotLightBuffer;
    struct {
        HGBufferVLK placementMatrix;
        HGBufferVLK boneMatrix;
        HGBufferVLK m2Colors;
        HGBufferVLK textureWeights;
        HGBufferVLK textureMatrices;
        HGBufferVLK modelFragmentDatas;

        HGBufferVLK m2InstanceData;
        HGBufferVLK meshWideBlocks;
        HGBufferVLK meshWideBlocksBindless;
        HGBufferVLK projectiveData;
    } m2Buffers;
    struct {
        HGBufferVLK waterfallCommon;
        HGBufferVLK waterfallBindless;
    } m2WaterfallBuffer;

    struct {
        HGBufferVLK waterLiquidBuffer;
        HGBufferVLK magmaLiquidBuffer;
        HGBufferVLK mercuryLiquidBuffer;
        HGBufferVLK fogLiquidBuffer;
        HGBufferVLK leyLineLiquidBuffer;
        HGBufferVLK felLiquidBuffer;
        HGBufferVLK swampLiquidBuffer;
        HGBufferVLK azeritheLiquidBuffer;
//        HGBufferVLK dataLiquidBuffer;

        HGBufferVLK liquidBindlessBuffer;
        HGBufferVLK liquidInstanceBuffer;
    } liquidBuffers;

    struct {
        HGBufferVLK adtMeshWideVSPSes;
        HGBufferVLK adtMeshWidePSes;
        HGBufferVLK adtInstanceDatas;
    } adtBuffers;

    struct {
        HGBufferVLK wmoGroupInteriorData;
        HGBufferVLK wmoPlacementMats;
        HGBufferVLK wmoMeshWideVSes;
        HGBufferVLK wmoMeshWidePSes;
        HGBufferVLK wmoMeshWideBindless;
        HGBufferVLK wmoPerMeshData;
    } wmoBuffers;

    HGBufferVLK uboBuffer;

    HGBufferVLK m_vboQuad;
    HGBufferVLK m_iboQuad;

    HGBufferVLK m_iboBBox;

    HGBufferVLK m_particleIndexBuffer;

    HGVertexBufferBindings m_drawQuadVao = nullptr;
    HGVertexBufferBindings m_drawBBoxVao = nullptr;

    HGBufferVLK m_vboSpot;
    HGBufferVLK m_iboSpot;
    HGBufferVLK m_iboSpotLine;

    // GPU-indirect draw path: owns the culling buffers/materials and the per-frame
    // indirect draw commands for M2s and WMO groups (opaque + transparent).
    BindlessIndirectDrawsVLK m_gpuDraws;

    // GPU M2 animation compute path (bones evaluated on GPU when Config::useGpuAnimation)
    GpuM2AnimationVLK m_gpuM2Animation;

    // Liquid render mode for the scene-wide descriptor set: 0=below, 1=above, 2=all
    HGBufferVLK m_renderModeBuffer;

    // AABB debug wireframe rendering (drawM2BB / drawWmoBB), reusing the GPU-culling AABB buffer
    HGBufferVLK m_iboAabbDebugLine;
    HGVertexBufferBindings m_drawAabbDebugVao = nullptr;
    std::shared_ptr<CBufferChunkVLK<mathfu::vec4_packed>> m_aabbDebugColor;
    std::shared_ptr<ISimpleMaterialVLK> m_aabbDebugMat;

    // Single-box highlight for the currently selected object (drawn regardless of drawM2BB/drawWmoBB)
    HGBufferVLK m_selectedAABBBuffer;
    std::shared_ptr<CBufferChunkVLK<mathfu::vec4_packed>> m_selectedAABBColor;
    std::shared_ptr<ISimpleMaterialVLK> m_selectedAABBMat;
    bool m_selectedAABBValid = false;
    CAaBox m_selectedAABBData;

    void createAABBDebugMaterials();
    void drawAABBDebug(CmdBufRecorder &frameBufCmd, const GpuIndirectDrawsVLK::FrameDrawData &gpuFrameDraws);
    void updateSelectedAABB(const std::shared_ptr<MapRenderPlan> &framePlan);

    // Async GPU object-id pick readback: one small host-visible, persistently-mapped buffer per
    // frame-in-flight slot. A copy is queued on the frame a pick is requested; the result is decoded
    // several frames later, once that slot's prior GPU work is known-complete (piggybacking on the
    // engine's existing frame-in-flight pacing rather than introducing a dedicated wait).
    std::array<VkBuffer, IDevice::MAX_FRAMES_IN_FLIGHT> m_pickResultBuffer = {};
    std::array<VmaAllocation, IDevice::MAX_FRAMES_IN_FLIGHT> m_pickResultAllocation = {};
    std::array<void*, IDevice::MAX_FRAMES_IN_FLIGHT> m_pickResultMapped = {};
    std::array<bool, IDevice::MAX_FRAMES_IN_FLIGHT> m_pickResultPending = {};
    // Whether the pending pick queued for this frame-in-flight slot was a passive hover peek
    // (see MapSceneParams::pickIsHoverPeek) — read back alongside m_pickResultPending when resolving.
    std::array<bool, IDevice::MAX_FRAMES_IN_FLIGHT> m_pickResultIsHoverPeek = {};

    // Persists across frames until the next pick request resolves (MapRenderPlan is rebuilt every
    // frame, so the current selection is copied into it each frame from here).
    bool m_hasSelectedM2 = false;
    M2ObjId m_selectedM2 = (M2ObjId)0;
    WMOObjId m_selectedWMO = emptyWMO;
    int m_selectedWMOGroupNum = -1;

    // Same as above, but for passive hover peeks — never drives the selection AABB highlight.
    bool m_hasHoveredM2 = false;
    M2ObjId m_hoveredM2 = (M2ObjId)0;
    WMOObjId m_hoveredWMO = emptyWMO;
    int m_hoveredWMOGroupNum = -1;

    void setupObjectPicking();
    void resolvePendingPickResult();

    std::vector<std::reference_wrapper<HGBufferVLK>> allBuffers = {};

    HGVertexBufferBindings m_drawSpotVao = nullptr;
    HGVertexBufferBindings m_drawSpotVaoLine = nullptr;

    std::shared_ptr<GBufferChunkDynamicVersionedVLK<sceneWideBlockVSPS>> sceneWideChunk;
    std::shared_ptr<GDescriptorSet> sceneWideDS = nullptr;
    std::shared_ptr<GDescriptorSet> gBufferDataDS = nullptr;

    std::shared_ptr<ISimpleMaterialVLK> g_m2Material = nullptr;
    std::shared_ptr<GDescriptorSet> m2BufferOneDS = nullptr;
    std::shared_ptr<GDescriptorSet> m2TextureDS = nullptr;
    std::shared_ptr<BindlessTextureHolder> m2TextureHolder = nullptr;

    std::shared_ptr<GDescriptorSet> m2WaterfallBufferDS = nullptr;
    std::shared_ptr<GDescriptorSet> m2WaterfallTextureDS = nullptr;
    std::shared_ptr<BindlessTextureHolder> m2WaterfallTextureHolder = nullptr;

    std::shared_ptr<ISimpleMaterialVLK> g_adtMaterial = nullptr;
    std::shared_ptr<GDescriptorSet> adtLayerTextureDS = nullptr;
    std::shared_ptr<BindlessTextureHolder> adtLayerTextureHolder = nullptr;
    std::shared_ptr<GDescriptorSet> adtHeightLayerTextureDS = nullptr;
    std::shared_ptr<BindlessTextureHolder> adtHeightLayerTextureHolder = nullptr;
    std::shared_ptr<GDescriptorSet> adtAlphaTextureDS = nullptr;
    std::shared_ptr<BindlessTextureHolder> adtAlphaTextureHolder = nullptr;

    std::shared_ptr<ISimpleMaterialVLK> g_wmoMaterial = nullptr;
    std::shared_ptr<GDescriptorSet> wmoBufferOneDS = nullptr;
    std::shared_ptr<GDescriptorSet> wmoTexturesDS = nullptr;
    std::shared_ptr<BindlessTextureHolder> wmoTextureHolder = nullptr;

    std::shared_ptr<ISimpleMaterialVLK> g_waterMaterial = nullptr;
    std::shared_ptr<GDescriptorSet> waterDataDS = nullptr;
    std::shared_ptr<GDescriptorSet> waterTexturesDS = nullptr;
    std::shared_ptr<BindlessTextureHolder> waterTextureHolder = nullptr;
    // Slot in waterTexturesDS holding the black placeholder texture; used for liquid
    // sampler slots that have no texture bound this frame.
    std::shared_ptr<BindlessTexture> waterFallbackTexture = nullptr;

    std::shared_ptr<GRenderPassVLK> m_forwardRenderPass;
    std::shared_ptr<GRenderPassVLK> m_gBufferPass;
    std::shared_ptr<GRenderPassVLK> m_shadowPass;

    std::shared_ptr<MapRenderPlan> m_lastCreatedPlan = nullptr;

    HGVertexBufferBindings m_emptyM2VAO = nullptr;
    HGVertexBufferBindings m_emptyADTVAO = nullptr;
    HGVertexBufferBindings m_emptyM2ParticleVAO = nullptr;
    // Pull-model particle VAO for the GPU particle path (index buffer only)
    HGVertexBufferBindings m_emptyM2ParticleGpuVAO = nullptr;
    // Pull-model ribbon VAO for the GPU ribbon path (index buffer only)
    HGVertexBufferBindings m_emptyM2RibbonGpuVAO = nullptr;
    HGVertexBufferBindings m_emptyM2RibbonVAO = nullptr;
    HGVertexBufferBindings m_emptyPortalVAO = nullptr;
    HGVertexBufferBindings m_emptySkyVAO = nullptr;
    HGVertexBufferBindings m_emptyWMOVAO = nullptr;
    HGVertexBufferBindings m_emptyWaterVAO = nullptr;


    using RendererViewClass = RenderViewDeferredVLK;
    std::shared_ptr<RendererViewClass> defaultView;

    // Secondary command buffers for the liquid transparent passes: the transparent draws
    // (indirect + direct) are identical between the below-liquid and above-liquid passes —
    // only the render-mode SSBO differs — so they are recorded once per frame per render
    // target and then executed in both passes via vkCmdExecuteCommands.
    // Indexed by [renderTargetIndex][frameNumber % MAX_FRAMES_IN_FLIGHT].
    std::vector<std::array<std::shared_ptr<GCommandBuffer>, IDevice::MAX_FRAMES_IN_FLIGHT>> m_liquidSecondaryCmdBufs;

    std::shared_ptr<GCommandBuffer> recordLiquidTransparentSecondary(
        const std::shared_ptr<RendererViewClass> &currentView,
        uint32_t renderTargetIndex,
        const std::shared_ptr<framebased::vector<HGSortableMesh>> &transparentMeshes,
        const GpuIndirectDrawsVLK::FrameDrawData &gpuFrameDraws);

    MeshCount lastMeshCount;

    void createM2GlobalMaterialData();
    void createWMOGlobalMaterialData();
    void createADTGlobalMaterialData();
    void createWaterGlobalMaterialData();
    void createM2WaterfallGlobalMaterialData();


    std::mt19937_64 eng; //Use the 64-bit Mersenne Twister 19937 generator
    //and seed it with entropy.
    std::uniform_int_distribution<unsigned long long> idDistr;
    robin_hood::unordered_flat_map<uint32_t, std::weak_ptr<ISimpleMaterialVLK>> m_m2MatCacheId;
    robin_hood::unordered_flat_map<uint32_t, std::weak_ptr<ISimpleMaterialVLK>> m_wmoMatCacheId;

    uint32_t generateUniqueWMOMatId() {
        uint32_t random;
        do {
            random = idDistr(eng);
        } while (!m_wmoMatCacheId[random].expired());

        return random;
    }

    uint32_t generateUniqueM2MatId() {
        uint32_t random;
        do {
            random = idDistr(eng);
        } while (!m_m2MatCacheId[random].expired());

        return random;
    }

public:
    const std::shared_ptr<ISimpleMaterialVLK> &getGlobalADTMaterial() const {return g_adtMaterial;};
    const std::shared_ptr<ISimpleMaterialVLK> &getGlobalM2Material() const {return g_m2Material;};
    const std::shared_ptr<ISimpleMaterialVLK> &getGlobalWMOMaterial() const {return g_wmoMaterial;};
    const std::shared_ptr<ISimpleMaterialVLK> &getGlobalWaterMaterial() const {return g_waterMaterial;};

    HGVertexBufferBindings getDefaultADTVao() const   {return m_emptyADTVAO;};
    HGVertexBufferBindings getDefaultM2Vao() const    {return m_emptyM2VAO;};
    HGVertexBufferBindings getDefaultWMOVao() const   {return m_emptyWMOVAO;};
    HGVertexBufferBindings getDefaultWaterVao() const {return m_emptyWaterVAO;};

    void
    drawOpaque(CmdBufRecorder &frameBufCmd,
               COpaqueMeshCollectorBindlessVLK *l_opaqueMeshes);
};

#endif //AWEBWOWVIEWERCPP_MAPSCENERENDERVISVLK_H
