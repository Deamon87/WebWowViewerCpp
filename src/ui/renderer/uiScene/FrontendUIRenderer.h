//
// Created by Deamon on 12.12.22.
//

#ifndef AWEBWOWVIEWERCPP_FRONTENDUIRENDERER_H
#define AWEBWOWVIEWERCPP_FRONTENDUIRENDERER_H

#include "../../../../wowViewerLib/src/renderer/IRenderParameters.h"
#include "ImGUIPlan.h"
#include "../../../../wowViewerLib/src/engine/shader/ShaderDefinitions.h"
#include "IFrontendUIBufferCreate.h"

static const std::array<GBufferBinding, 3> imguiBindings = {{
    {+imguiShader::Attribute::Position, 2, GBindingType::GFLOAT, false, sizeof(ImDrawVert), IM_OFFSETOF(ImDrawVert, pos)},
    {+imguiShader::Attribute::UV, 2, GBindingType::GFLOAT, false, sizeof(ImDrawVert), IM_OFFSETOF(ImDrawVert, uv)},
    {+imguiShader::Attribute::Color, 4, GBindingType::GUNSIGNED_BYTE, true, sizeof(ImDrawVert), IM_OFFSETOF(ImDrawVert, col)},
}};


class FrontendUIRenderer : public IRendererParameters<ImGuiFramePlan::ImGUIParam, ImGuiFramePlan::EmptyPlan>, public IFrontendUIBufferCreate  {
public:
    virtual ~FrontendUIRenderer() = default;
    void processFramePlan();
private:
    HGDevice m_device = nullptr;
    std::shared_ptr<IBufferChunk<ImgUI::modelWideBlockVS>> m_imguiUbo = nullptr;
};

//typedef FrameInputParams<ImGuiFramePlan::EmptyPlan, ImGuiFramePlan::ImGUIParam> FrontendUIInputParams;
typedef FrameInputParams<ImGuiFramePlan::ImGUIParam> FrontendUIInputParams;

#endif //AWEBWOWVIEWERCPP_FRONTENDUIRENDERER_H
