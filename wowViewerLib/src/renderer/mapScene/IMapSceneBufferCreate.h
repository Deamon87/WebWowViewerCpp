//
// Created by Deamon on 22.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IMAPSCENEBUFFERCREATE_H
#define AWEBWOWVIEWERCPP_IMAPSCENEBUFFERCREATE_H

#include <memory>
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
        mathfu::vec3_packed pos;
        mathfu::vec3_packed normal;
        mathfu::vec4_packed mccv;
        mathfu::vec4_packed mclv;
    }
);

PACK(
    struct LiquidVertexFormat {
        mathfu::vec4_packed pos_transp;
        mathfu::vec2_packed uv;
    }
);

static const size_t MAX_PARTICLES_PER_EMITTER = 2000;

class IMapSceneBufferCreate {
public:
    virtual ~IMapSceneBufferCreate() = default;

//-------------------------------------
//  Buffer creation
//-------------------------------------

    virtual HGVertexBufferBindings createADTVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) = 0;
    virtual HGVertexBufferBindings createWmoVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer, const std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> &ambientBuffer) = 0;
    virtual HGVertexBufferBindings createM2VAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) = 0;
    virtual HGVertexBufferBindings createM2ParticleVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) = 0;
    virtual HGVertexBufferBindings createM2RibbonVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) = 0;
    virtual HGVertexBufferBindings createWaterVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) = 0;
    virtual HGVertexBufferBindings createSkyVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) = 0;
    virtual HGVertexBufferBindings createPortalVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) = 0;

    virtual HGVertexBuffer createPortalVertexBuffer(int sizeInBytes) = 0;
    virtual HGIndexBuffer  createPortalIndexBuffer(int sizeInBytes) = 0;

    virtual HGVertexBuffer createM2VertexBuffer(int sizeInBytes) = 0;

    virtual HGIndexBuffer  createM2IndexBuffer(int sizeInBytes) = 0;

    virtual HGIndexBuffer  getOrCreateM2ParticleIndexBuffer() = 0;

    virtual HGVertexBuffer createM2ParticleVertexBuffer(int sizeInBytes) = 0;
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

    virtual std::shared_ptr<IM2ModelData> createM2ModelMat(int bonesCount, int m2ColorsCount, int textureWeightsCount, int textureMatricesCount) = 0;

    virtual std::shared_ptr<IM2Material> createM2Material(const std::shared_ptr<IM2ModelData> &m2ModelData,
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

    virtual std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> createWMOWideChunk() = 0;
    virtual std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> createWMOGroupAmbientChunk() = 0;

    virtual std::shared_ptr<IWMOMaterial> createWMOMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
                                                            const PipelineTemplate &pipelineTemplate,
                                                            const WMOMaterialTemplate &wmoMaterialTemplate) = 0;

    virtual std::shared_ptr<IWaterMaterial> createWaterMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
                                                               const PipelineTemplate &pipelineTemplate,
                                                               const WaterMaterialTemplate &waterMaterialTemplate) = 0;

    virtual std::shared_ptr<IPortalMaterial> createPortalMaterial(const PipelineTemplate &pipelineTemplate) = 0;
//-------------------------------------
//  Mesh creation
//-------------------------------------

    virtual HGMesh createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) = 0;
    virtual HGSortableMesh createSortableMesh(gMeshTemplate &meshTemplate, const HMaterial &material, int priorityPlane) = 0;
    virtual HGMesh createAdtMesh(gMeshTemplate &meshTemplate,  const std::shared_ptr<IADTMaterial> &material) = 0;
    virtual HGM2Mesh createM2Mesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2Material> &material, int layer, int priorityPlane) = 0;
    virtual HGSortableMesh createWaterMesh(gMeshTemplate &meshTemplate, const HMaterial &material, int priorityPlane) = 0;
    virtual HGSortableMesh createWMOMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IWMOMaterial> &material, const std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> &ambientBuffer) = 0;
    virtual HGM2Mesh createM2WaterfallMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2WaterFallMaterial> &material, int layer, int priorityPlane) = 0;
};
typedef std::shared_ptr<IMapSceneBufferCreate> HMapSceneBufferCreate;

#endif //AWEBWOWVIEWERCPP_IMAPSCENEBUFFERCREATE_H
