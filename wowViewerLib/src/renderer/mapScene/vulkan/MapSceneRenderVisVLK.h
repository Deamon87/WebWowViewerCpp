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
#include "../../../engine/objects/scenes/EntityActorsFactory.h"
#include "view/RenderViewDeferredVLK.h"
#include "MapSceneRenderBindlessVLK.h"

class MapSceneRenderVisBufferVLK : public MapSceneRenderBindlessVLK {
    friend class COpaqueMeshCollectorBindlessVLK;
public:
    explicit MapSceneRenderVisBufferVLK(const HGDeviceVLK &hDevice, Config *config);
    ~MapSceneRenderVisBufferVLK() override = default;

    std::unique_ptr<IRenderFunction> update(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams, const std::shared_ptr<MapRenderPlan> &framePlan) override;

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

    std::shared_ptr<IWmoModelData> createWMOWideChunk(int groupNum)  override;
    std::shared_ptr<IBufferChunk<WMO::GroupInteriorData>> createWMOGroupAmbientChunk() override;

    std::shared_ptr<IWMOMaterial> createWMOMaterial(const std::shared_ptr<IWmoModelData> &wmoModelWide,
                                                    const PipelineTemplate &pipelineTemplate,
                                                    const WMOMaterialTemplate &wmoMaterialTemplate) override;
    std::shared_ptr<IWaterMaterial> createWaterMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
                                                        const PipelineTemplate &pipelineTemplate,
                                                        const WaterMaterialTemplate &waterMaterialTemplate) override;

    std::shared_ptr<ISkyMeshMaterial> createSkyMeshMaterial(const PipelineTemplate &pipelineTemplate) override;

    std::shared_ptr<IPortalMaterial> createPortalMaterial(const PipelineTemplate &pipelineTemplate) override;

    std::shared_ptr<IRenderView> createRenderView(bool createOutput) override;
protected:
    virtual std::shared_ptr<ISimpleMaterialVLK> getM2StaticMaterial(const PipelineTemplate &pipelineTemplate) ;
    virtual std::shared_ptr<ISimpleMaterialVLK> getWMOStaticMaterial(const PipelineTemplate &pipelineTemplate);

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

#endif //AWEBWOWVIEWERCPP_MAPSCENERENDERVISVLK_H
