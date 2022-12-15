//
// Created by Deamon on 12.12.22.
//

#ifndef AWEBWOWVIEWERCPP_FRONTENDUIRENDERER_H
#define AWEBWOWVIEWERCPP_FRONTENDUIRENDERER_H

#include "../../../../wowViewerLib/src/renderer/IRenderParameters.h"
#include "ImGUIPlan.h"
#include "buffers/IVertexBufferDynamicImgui.h"

class FrontendUIRenderer : public IRendererParameters<ImGuiFramePlan::EmptyPlan, ImGuiFramePlan::ImGUIParam>  {
public:
    virtual HIVertexBufferDynamicImgui allocateDynamicVertexBuffer(int size) = 0;
    virtual HIVertexBufferDynamicImgui allocateDynamicIndexBuffer(int size) = 0;

//    virtual HGMesh allocateImguiMesh(eshTemplate)
};

typedef FrameInputParams<ImGuiFramePlan::EmptyPlan, ImGuiFramePlan::ImGUIParam> FrontendUIInputParams;

#endif //AWEBWOWVIEWERCPP_FRONTENDUIRENDERER_H
