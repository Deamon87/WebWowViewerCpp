//
// Created by Deamon on 11/26/2022.
//

#ifndef AWEBWOWVIEWERCPP_MAPSCENERENDERER_H
#define AWEBWOWVIEWERCPP_MAPSCENERENDERER_H


#include "../IRenderer.h"
#include "../IRenderParameters.h"
#include "IMapSceneBufferCreate.h"
#include "MapSceneParams.h"
#include "MapScenePlan.h"

#include "../../gapi/interface/buffers/IBufferVersioned.h"
#include <ShaderDefinitions.h>
#include "../../engine/algorithms/FrameCounter.h"

static const std::vector<GBufferBinding> staticWMOBindings = {{
    {+wmoShader::Attribute::aPosition, 3, GBindingType::GFLOAT, false,          sizeof(WMOVertex), offsetof(WMOVertex, pos) },
    {+wmoShader::Attribute::aNormal, 3, GBindingType::GFLOAT, false,            sizeof(WMOVertex), offsetof(WMOVertex, normal)},
    {+wmoShader::Attribute::aTexCoord, 2, GBindingType::GFLOAT, false,          sizeof(WMOVertex), offsetof(WMOVertex, textCoordinate)},
    {+wmoShader::Attribute::aTexCoord2, 2, GBindingType::GFLOAT, false,         sizeof(WMOVertex), offsetof(WMOVertex, textCoordinate2)},
    {+wmoShader::Attribute::aTexCoord3, 2, GBindingType::GFLOAT, false,         sizeof(WMOVertex), offsetof(WMOVertex, textCoordinate3)},
    {+wmoShader::Attribute::aTexCoord4, 2, GBindingType::GFLOAT, false,         sizeof(WMOVertex), offsetof(WMOVertex, textCoordinate4)},
    {+wmoShader::Attribute::aColor, 4, GBindingType::GUNSIGNED_BYTE, true,      sizeof(WMOVertex), offsetof(WMOVertex, color)},
    {+wmoShader::Attribute::aColor2, 4, GBindingType::GUNSIGNED_BYTE, true,     sizeof(WMOVertex), offsetof(WMOVertex, color2)},
    {+wmoShader::Attribute::aColorSecond, 4, GBindingType::GUNSIGNED_BYTE, true,sizeof(WMOVertex), offsetof(WMOVertex, colorSecond)}
}};
// static const std::vector<GBufferBinding> staticWmoGroupAmbient = {{
//     {+wmoShader::Attribute::wmoAmbient, 4, GBindingType::GFLOAT, false,          sizeof(mathfu::vec4_packed), 0},
// }};

static const std::vector<GBufferBinding> staticWaterBindings = {{
    {+waterShader::Attribute::aPositionTransp, 4, GBindingType::GFLOAT, false, sizeof(LiquidVertexFormat), offsetof(LiquidVertexFormat, pos_transp)},
    {+waterShader::Attribute::aTexCoord, 2, GBindingType::GFLOAT, false, sizeof(LiquidVertexFormat), offsetof(LiquidVertexFormat, uv)}
}};

static const std::vector<GBufferBinding> staticM2Bindings = {{
    {+m2Shader::Attribute::aPosition,   3, GBindingType::GFLOAT,         false, sizeof(M2Vertex), offsetof(M2Vertex, pos) },
    {+m2Shader::Attribute::aNormal,     3, GBindingType::GFLOAT,         false, sizeof(M2Vertex), offsetof(M2Vertex, normal)}, // normal
    {+m2Shader::Attribute::bones,       4, GBindingType::GUNSIGNED_BYTE, false, sizeof(M2Vertex), offsetof(M2Vertex, bone_indices)},  // bones
    {+m2Shader::Attribute::boneWeights, 4, GBindingType::GUNSIGNED_BYTE, true,  sizeof(M2Vertex), offsetof(M2Vertex, bone_weights)},  // bonesWeight
    {+m2Shader::Attribute::aTexCoord,   2, GBindingType::GFLOAT,         false, sizeof(M2Vertex), offsetof(M2Vertex, tex_coords)}, // texcoord
    {+m2Shader::Attribute::aTexCoord2,  2, GBindingType::GFLOAT,         false, sizeof(M2Vertex), offsetof(M2Vertex, tex_coords[1])} // texcoord
}};

static_assert(sizeof(M2Vertex) == 48);

static const std::vector<GBufferBinding> skyConusBinding = {{
    {+skyConus::Attribute::aPosition, 4, GBindingType::GFLOAT, false, 0, 0},
}};

static const std::vector<GBufferBinding> fullScreenQuad = {{
    {+drawQuad::Attribute::position, 2, GBindingType::GFLOAT, false, 0, 0},
}};

static const std::vector<GBufferBinding> staticM2ParticleBindings = {{
    {+m2ParticleShader::Attribute::aPosition,  3, GBindingType::GFLOAT, false, sizeof(ParticleBuffStruct), offsetof(ParticleBuffStruct, position) },
    {+m2ParticleShader::Attribute::aColor,     4, GBindingType::GFLOAT, false, sizeof(ParticleBuffStruct), offsetof(ParticleBuffStruct, color)},
    {+m2ParticleShader::Attribute::aTexcoord0, 2, GBindingType::GFLOAT, false, sizeof(ParticleBuffStruct), offsetof(ParticleBuffStruct, textCoord0)},
    {+m2ParticleShader::Attribute::aTexcoord1, 2, GBindingType::GFLOAT, false, sizeof(ParticleBuffStruct), offsetof(ParticleBuffStruct, textCoord1)},
    {+m2ParticleShader::Attribute::aTexcoord2, 2, GBindingType::GFLOAT, false, sizeof(ParticleBuffStruct), offsetof(ParticleBuffStruct, textCoord2)},
    {+m2ParticleShader::Attribute::aAlphaCutoff, 1, GBindingType::GFLOAT, false, sizeof(ParticleBuffStruct), offsetof(ParticleBuffStruct, alphaCutoff)},
}};

static std::vector<GBufferBinding> staticM2RibbonBindings = {{
    {+ribbonShader::Attribute::aPosition, 3, GBindingType::GFLOAT, false, sizeof(CRibbonVertex), offsetof(CRibbonVertex, pos) }, // 0
    {+ribbonShader::Attribute::aColor, 4, GBindingType::GUNSIGNED_BYTE, true, sizeof(CRibbonVertex), offsetof(CRibbonVertex, diffuseColor)}, // 12
    {+ribbonShader::Attribute::aTexcoord0, 2, GBindingType::GFLOAT, false, sizeof(CRibbonVertex), offsetof(CRibbonVertex, texCoord)}, // 16
 //24
}};

static std::vector<GBufferBinding> adtVertexBufferBinding = {{
    {+adtShader::Attribute::aHeight, 1,    GBindingType::GFLOAT, false,      sizeof(AdtVertex), offsetof(AdtVertex, height)},
    {+adtShader::Attribute::aNormal, 4,    GBindingType::GSIGNED_BYTE, true,      sizeof(AdtVertex), offsetof(AdtVertex, normal)},
    {+adtShader::Attribute::aColor, 4,     GBindingType::GUNSIGNED_BYTE, true,      sizeof(AdtVertex), offsetof(AdtVertex, mccv)},
    {+adtShader::Attribute::aVertexLighting, 4, GBindingType::GUNSIGNED_BYTE, true, sizeof(AdtVertex), offsetof(AdtVertex, mclv)},
}};

static std::vector<GBufferBinding> adtVertexBufferLODBinding = {{
    {+adtLodShader::Attribute::aHeight, 1, GBindingType::GFLOAT, false, 8, 0 },
    {+adtLodShader::Attribute::aIndex, 1, GBindingType::GFLOAT, false, 8, 4}
}};

static std::vector<GBufferBinding> drawPortalBindings = {{
     {+drawPortalShader::Attribute::aPosition, 3, GBindingType::GFLOAT, false, 12, 0}, // 0
 }};

struct MeshCount {
    int commonMesh = 0;
    int m2Mesh = 0;
    int wmoMesh = 0;
    int adtMesh = 0;
};

struct RenderingMatAndSceneSize {
    HCameraMatrices renderingMat;
    int width;
    int height;
};

class MapSceneRenderer : public IRenderer, public IMapSceneBufferCreate, public IRendererParameters<MapSceneParams, MapRenderPlan>  {
public:
    MapSceneRenderer(Config *config) : m_config(config) {};
    ~MapSceneRenderer() override = default;

    std::shared_ptr<MapRenderPlan> processCulling(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams) override;

    void collectMeshes(const std::shared_ptr<MapRenderPlan> &renderPlan,
                       COpaqueMeshCollector &opaqueMeshCollector,
                       COpaqueMeshCollector &skyOpaqueMeshCollector,
                       const std::shared_ptr<framebased::vector<HGSortableMesh>> &htransparentMeshes,
                       const std::shared_ptr<framebased::vector<HGSortableMesh>> &hSkyTransparentMeshes);

    void updateSceneWideChunk(const std::shared_ptr<IBufferChunkVersioned<sceneWideBlockVSPS>> &sceneWideChunk,
                              const std::vector<RenderingMatAndSceneSize> &renderingMatricesAndSizes,
                              const HFrameDependantData &fdd,
                              bool isVulkan,
                              animTime_t sceneTime);

    virtual std::shared_ptr<IRenderView> createRenderView(bool createOutput) = 0;
protected:
    Config *m_config;
};


//typedef FrameInputParams<MapRenderPlan, void> MapSceneRendererInputParams;
typedef FrameInputParams<MapSceneParams> MapSceneRendererInputParams;
typedef std::shared_ptr<MapSceneRenderer> HMapSceneRenderer;

#endif //AWEBWOWVIEWERCPP_MAPSCENERENDERER_H
