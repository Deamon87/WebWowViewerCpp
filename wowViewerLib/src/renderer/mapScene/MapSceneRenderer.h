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

static const std::array<GBufferBinding, 9> staticWMOBindings = {{
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

static const std::array<GBufferBinding, 2> staticWaterBindings = {{
    {+waterShader::Attribute::aPositionTransp, 4, GBindingType::GFLOAT, false, sizeof(LiquidVertexFormat), offsetof(LiquidVertexFormat, pos_transp)},
    {+waterShader::Attribute::aTexCoord, 2, GBindingType::GFLOAT, false, sizeof(LiquidVertexFormat), offsetof(LiquidVertexFormat, uv)}
}};

static const std::array<GBufferBinding, 6> staticM2Bindings = {{
    {+m2Shader::Attribute::aPosition, 3, GBindingType::GFLOAT, false, 48, 0 },
    {+m2Shader::Attribute::boneWeights, 4, GBindingType::GUNSIGNED_BYTE, true, 48, 12},  // bonesWeight
    {+m2Shader::Attribute::bones, 4, GBindingType::GUNSIGNED_BYTE, false, 48, 16},  // bones
    {+m2Shader::Attribute::aNormal, 3, GBindingType::GFLOAT, false, 48, 20}, // normal
    {+m2Shader::Attribute::aTexCoord, 2, GBindingType::GFLOAT, false, 48, 32}, // texcoord
    {+m2Shader::Attribute::aTexCoord2, 2, GBindingType::GFLOAT, false, 48, 40} // texcoord
}};

static const std::array<GBufferBinding, 1> skyConusBinding = {{
    {+drawQuad::Attribute::position, 4, GBindingType::GFLOAT, false, 0, 0},
}};

class MapSceneRenderer : public IRenderer, public IMapSceneBufferCreate, public IRendererParameters<MapSceneParams, MapRenderPlan>  {
public:
    MapSceneRenderer() = default;
    ~MapSceneRenderer() override = default;

    std::shared_ptr<MapRenderPlan> processCulling(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams) override;;
};

//typedef FrameInputParams<MapRenderPlan, void> MapSceneRendererInputParams;
typedef FrameInputParams<MapSceneParams> MapSceneRendererInputParams;
typedef std::shared_ptr<MapSceneRenderer> HMapSceneRenderer;

#endif //AWEBWOWVIEWERCPP_MAPSCENERENDERER_H
