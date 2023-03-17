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

#include "../../engine/shader/ShaderDefinitions.h"
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

static const std::vector<GBufferBinding> staticWaterBindings = {{
    {+waterShader::Attribute::aPositionTransp, 4, GBindingType::GFLOAT, false, sizeof(LiquidVertexFormat), offsetof(LiquidVertexFormat, pos_transp)},
    {+waterShader::Attribute::aTexCoord, 2, GBindingType::GFLOAT, false, sizeof(LiquidVertexFormat), offsetof(LiquidVertexFormat, uv)}
}};

static const std::vector<GBufferBinding> staticM2Bindings = {{
    {+m2Shader::Attribute::aPosition, 3, GBindingType::GFLOAT, false, sizeof(M2Vertex), 0 },
    {+m2Shader::Attribute::boneWeights, 4, GBindingType::GUNSIGNED_BYTE, true, sizeof(M2Vertex), 12},  // bonesWeight
    {+m2Shader::Attribute::bones, 4, GBindingType::GUNSIGNED_BYTE, false, sizeof(M2Vertex), 16},  // bones
    {+m2Shader::Attribute::aNormal, 3, GBindingType::GFLOAT, false, sizeof(M2Vertex), 20}, // normal
    {+m2Shader::Attribute::aTexCoord, 2, GBindingType::GFLOAT, false, sizeof(M2Vertex), 32}, // texcoord
    {+m2Shader::Attribute::aTexCoord2, 2, GBindingType::GFLOAT, false, sizeof(M2Vertex), 40} // texcoord
}};

static_assert(sizeof(M2Vertex) == 48);

static const std::vector<GBufferBinding> skyConusBinding = {{
    {+drawQuad::Attribute::position, 4, GBindingType::GFLOAT, false, 0, 0},
}};

class MapSceneRenderer : public IRenderer, public IMapSceneBufferCreate, public IRendererParameters<MapSceneParams, MapRenderPlan>  {
public:
    MapSceneRenderer(Config *config) : m_config(config) {};
    ~MapSceneRenderer() override = default;

    std::shared_ptr<MapRenderPlan> processCulling(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams) override;

    std::tuple<
        std::shared_ptr<std::vector<HGMesh>>,
        std::shared_ptr<std::vector<HGMesh>>
    > collectMeshes(const std::shared_ptr<MapRenderPlan> &renderPlan);
    void updateSceneWideChunk(const std::shared_ptr<IBufferChunk<sceneWideBlockVSPS>> &sceneWideChunk,
                              const HCameraMatrices &renderingMatrices,
                              const HFrameDependantData &fdd,
                              bool isVulkan);

private:
    FrameCounter mapProduceUpdateCounter;
    FrameCounter interiorViewCollectMeshCounter;
    FrameCounter exteriorViewCollectMeshCounter;
    FrameCounter m2CollectMeshCounter;
    FrameCounter sortMeshCounter;
    FrameCounter collectBuffersCounter;
    FrameCounter sortBuffersCounter;

    Config *m_config;
};

//typedef FrameInputParams<MapRenderPlan, void> MapSceneRendererInputParams;
typedef FrameInputParams<MapSceneParams> MapSceneRendererInputParams;
typedef std::shared_ptr<MapSceneRenderer> HMapSceneRenderer;

#endif //AWEBWOWVIEWERCPP_MAPSCENERENDERER_H
