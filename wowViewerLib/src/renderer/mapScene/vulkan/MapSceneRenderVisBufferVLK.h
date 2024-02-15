//
// Created by Deamon on 12/1/2022.
//

#ifndef AWEBWOWVIEWERCPP_MAPSCENERENDERVISVLK_H
#define AWEBWOWVIEWERCPP_MAPSCENERENDERVISVLK_H


#include "../MapSceneRenderer.h"
#include "../../../gapi/vulkan/GDeviceVulkan.h"
#include "../../../gapi/vulkan/buffers/GBufferChunkDynamicVersionedVLK.h"
#include "../materials/IMaterialStructs.h"
#include "passes/FFXGlowPassVLK.h"
#include "../../../gapi/vulkan/materials/ISimpleMaterialVLK.h"
#include "view/RenderViewForwardVLK.h"
#include "../../../gapi/vulkan/descriptorSets/bindless/BindlessTextureHolder.h"
#include "../../../engine/objects/scenes/EntityActorsFactory.h"
#include "view/RenderViewDeferredVLK.h"

class MapSceneRenderVisBufferVLK : public MapSceneRenderer {
    friend class COpaqueMeshCollectorBindlessVLK;
public:
    explicit MapSceneRenderVisBufferVLK(const HGDeviceVLK &hDevice, Config *config);
    ~MapSceneRenderVisBufferVLK() override = default;

    std::unique_ptr<IRenderFunction> update(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams, const std::shared_ptr<MapRenderPlan> &framePlan) override;
    inline static void drawMesh(CmdBufRecorder &cmdBuf, const HGMesh &mesh, CmdBufRecorder::ViewportType viewportType);

    std::shared_ptr<MapRenderPlan> getLastCreatedPlan() override;

//-------------------------------------
//  Buffer creation
//-------------------------------------
    HGVertexBufferBindings createADTVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) override;
    HGVertexBufferBindings createWmoVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer, const std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> &ambientBuffer) override;
    HGVertexBufferBindings createM2VAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) override;
    HGVertexBufferBindings createM2ParticleVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) override;
    HGVertexBufferBindings createM2RibbonVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) override;
    HGVertexBufferBindings createWaterVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) override;
    HGVertexBufferBindings createSkyVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) override;
    HGVertexBufferBindings createPortalVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) override;

    HGVertexBuffer createPortalVertexBuffer(int sizeInBytes) override;
    HGIndexBuffer  createPortalIndexBuffer(int sizeInBytes) override;

    HGVertexBuffer createM2VertexBuffer(int sizeInBytes) override;
    HGIndexBuffer  createM2IndexBuffer(int sizeInBytes) override;

    HGVertexBuffer createM2ParticleVertexBuffer(int sizeInBytes) override;
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
    std::shared_ptr<IM2ModelData> createM2ModelMat(int bonesCount, int m2ColorsCount, int textureWeightsCount, int textureMatricesCount) override;
    std::shared_ptr<IM2Material> createM2Material(const std::shared_ptr<IM2ModelData> &m2ModelData,
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

    std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> createWMOWideChunk() override;
    std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> createWMOGroupAmbientChunk() override;

    std::shared_ptr<IWMOMaterial> createWMOMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
                                                    const PipelineTemplate &pipelineTemplate,
                                                    const WMOMaterialTemplate &wmoMaterialTemplate) override;
    std::shared_ptr<IWaterMaterial> createWaterMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
                                                        const PipelineTemplate &pipelineTemplate,
                                                        const WaterMaterialTemplate &waterMaterialTemplate) override;

    std::shared_ptr<ISkyMeshMaterial> createSkyMeshMaterial(const PipelineTemplate &pipelineTemplate) override;

    std::shared_ptr<IPortalMaterial> createPortalMaterial(const PipelineTemplate &pipelineTemplate) override;

//-------------------------------------
//  Mesh creation
//-------------------------------------

    HGMesh createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) override;
    HGSortableMesh createSortableMesh(gMeshTemplate &meshTemplate, const HMaterial &material, int priorityPlane) override;
    HGMesh createAdtMesh(gMeshTemplate &meshTemplate,  const std::shared_ptr<IADTMaterial> &material) override;
    HGM2Mesh createM2Mesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2Material> &material, int layer, int priorityPlane) override;
    HGSortableMesh createWaterMesh(gMeshTemplate &meshTemplate, const HMaterial &material, int priorityPlane) override;
    HGMesh createWMOMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IWMOMaterial> &material, const std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> &ambientBuffer) override;
    HGM2Mesh createM2WaterfallMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2WaterFallMaterial> &material, int layer, int priorityPlane) override;

//--------------------------------------
// RenderView
//--------------------------------------

    std::shared_ptr<IRenderView> createRenderView(bool createOutput) override;

    std::shared_ptr<EntityFactory<GMeshVLK>> meshFactory = std::make_shared<EntityFactory<GMeshVLK>>();
private:
    std::shared_ptr<ISimpleMaterialVLK> getM2StaticMaterial(const PipelineTemplate &pipelineTemplate);
    std::shared_ptr<ISimpleMaterialVLK> getWMOStaticMaterial(const PipelineTemplate &pipelineTemplate);

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
    robin_hood::unordered_flat_map<PipelineTemplate, std::shared_ptr<ISimpleMaterialVLK>, PipelineTemplateHasher> m_wmoStaticMaterials;
private:
    HGDeviceVLK m_device;

    int m_width = 640;
    int m_height = 480;

    std::unique_ptr<FFXGlowPassVLK> glowPass;


    HGBufferVLK vboM2Buffer;
    HGBufferVLK vboM2ParticleBuffer;
    HGBufferVLK vboM2RibbonBuffer;
    HGBufferVLK vboPortalBuffer;

    HGBufferVLK vboAdtBuffer;
    HGBufferVLK vboWMOBuffer;
    HGBufferVLK vboWaterBuffer;
    HGBufferVLK vboSkyBuffer;

    HGBufferVLK iboBuffer;

    HGBufferVLK uboStaticBuffer;
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
    } m2Buffers;
    struct {
        HGBufferVLK waterfallCommon;
        HGBufferVLK waterfallBindless;
    } m2WaterfallBuffer;

    struct {
        HGBufferVLK waterDataBuffer;
        HGBufferVLK waterBindlessBuffer;
    } waterBuffer;

    struct {
        HGBufferVLK adtMeshWideVSPSes;
        HGBufferVLK adtMeshWidePSes;
        HGBufferVLK adtInstanceDatas;
    } adtBuffers;

    struct {
        HGBufferVLK wmoPlacementMats;
        HGBufferVLK wmoMeshWideVSes;
        HGBufferVLK wmoMeshWidePSes;
        HGBufferVLK wmoMeshWideBindless;
        HGBufferVLK wmoPerMeshData;
        HGBufferVLK wmoGroupAmbient;
    } wmoBuffers;

    HGBufferVLK uboBuffer;

    HGBufferVLK m_vboQuad;
    HGBufferVLK m_iboQuad;

    HGVertexBufferBindings m_drawQuadVao = nullptr;

    std::shared_ptr<GBufferChunkDynamicVersionedVLK<sceneWideBlockVSPS>> sceneWideChunk;
    std::shared_ptr<GDescriptorSet> sceneWideDS = nullptr;

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

    std::shared_ptr<GRenderPassVLK> m_opaqueRenderPass;
    std::shared_ptr<GRenderPassVLK> m_nonOpaquerenderPass;

    std::shared_ptr<MapRenderPlan> m_lastCreatedPlan = nullptr;

    HGVertexBufferBindings m_emptyM2VAO = nullptr;
    HGVertexBufferBindings m_emptyADTVAO = nullptr;
    HGVertexBufferBindings m_emptyM2ParticleVAO = nullptr;
    HGVertexBufferBindings m_emptyM2RibbonVAO = nullptr;
    HGVertexBufferBindings m_emptyPortalVAO = nullptr;
    HGVertexBufferBindings m_emptySkyVAO = nullptr;
    HGVertexBufferBindings m_emptyWMOVAO = nullptr;
    HGVertexBufferBindings m_emptyWaterVAO = nullptr;


    using RendererViewClass = RenderViewDeferredVLK;
    std::shared_ptr<RenderViewDeferredVLK> defaultView;

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

    std::shared_ptr<GRenderPassVLK> chooseRenderPass(const PipelineTemplate &pipelineTemplate);

};

class IM2ModelDataVisVLK : public IM2ModelData {
public:
    ~IM2ModelDataVisVLK() override = default;
    std::shared_ptr<GDescriptorSet> placementMatrixDS;
};



#endif //AWEBWOWVIEWERCPP_MAPSCENERENDERVISVLK_H
