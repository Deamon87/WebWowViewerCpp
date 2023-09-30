//
// Created by Deamon on 12.12.22.
//

#ifndef AWEBWOWVIEWERCPP_FRONTENDUIRENDERER_H
#define AWEBWOWVIEWERCPP_FRONTENDUIRENDERER_H

#include "../../../../wowViewerLib/src/renderer/IRenderParameters.h"
#include "ImGUIPlan.h"
#include "../../../../wowViewerLib/src/engine/shader/ShaderDefinitions.h"
#include "IFrontendUIBufferCreate.h"
#include "../../../../wowViewerLib/src/gapi/vulkan/buffers/CBufferChunkVLK.h"

static const std::array<GBufferBinding, 3> imguiBindings = {{
    {+imguiShader::Attribute::Position, 2, GBindingType::GFLOAT, false, sizeof(ImDrawVert), IM_OFFSETOF(ImDrawVert, pos)},
    {+imguiShader::Attribute::UV, 2, GBindingType::GFLOAT, false, sizeof(ImDrawVert), IM_OFFSETOF(ImDrawVert, uv)},
    {+imguiShader::Attribute::Color, 4, GBindingType::GUNSIGNED_BYTE, true, sizeof(ImDrawVert), IM_OFFSETOF(ImDrawVert, col)},
}};

static const PipelineTemplate s_imguiPipelineTemplate = {
    DrawElementMode::TRIANGLES,
    false,
    true,
    EGxBlendEnum::GxBlend_Alpha,
    false,
    true,
    0xFF
};


class FrontendUIRenderer : public IRendererParameters<ImGuiFramePlan::ImGUIParam, ImGuiFramePlan::EmptyPlan>, public IFrontendUIBufferCreate  {
public:
    FrontendUIRenderer(HGDevice m_device) : m_device(m_device) {

    }
    ~FrontendUIRenderer() override = default;

    std::shared_ptr<ImGuiFramePlan::EmptyPlan> processCulling(const std::shared_ptr<FrameInputParams<ImGuiFramePlan::ImGUIParam>> &frameInputParams) override {
        return nullptr;
    };
    std::shared_ptr<ImGuiFramePlan::EmptyPlan> getLastCreatedPlan() override { return nullptr; }
    HGSamplableTexture uploadFontTexture(unsigned char* pixels, int width, int height);
protected:
    HGDevice m_device = nullptr;
    HGSamplableTexture fontTexture;

    UiMaterialCache m_materialCache;
    UiMaterialCacheMap m_materialCacheIdMap;
    std::shared_ptr<CBufferChunkVLK<ImgUI::modelWideBlockVS>> m_imguiUbo = nullptr;

    void consumeFrameInput(const std::shared_ptr<FrameInputParams<ImGuiFramePlan::ImGUIParam>> &frameInputParams, std::vector<HGMesh> &meshes);

};

//typedef FrameInputParams<ImGuiFramePlan::EmptyPlan, ImGuiFramePlan::ImGUIParam> FrontendUIInputParams;
typedef FrameInputParams<ImGuiFramePlan::ImGUIParam> FrontendUIInputParams;

#endif //AWEBWOWVIEWERCPP_FRONTENDUIRENDERER_H
