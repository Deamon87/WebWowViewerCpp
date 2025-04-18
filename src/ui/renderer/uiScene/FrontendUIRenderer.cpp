//
// Created by Deamon on 12.12.22.
//

#include "FrontendUIRenderer.h"

#include "../../../../wowViewerLib/src/engine/algorithms/mathHelper.h"

//This should be called during Update/Draw stage, cause this stuff allocates buffers
void FrontendUIRenderer::consumeFrameInput(const std::shared_ptr<FrameInputParams<ImGuiFramePlan::ImGUIParam>> &frameInputParams, std::vector<HGMesh> &meshes) {

    auto frameParam = frameInputParams->frameParameters;

    auto draw_data = frameParam->getImData();

    int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0) {
        return;
    }
    ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)
    //Create projection matrix:
    auto uiScale = ImGui::GetIO().uiScale;
    float L = draw_data->DisplayPos.x * uiScale;
    float R = (draw_data->DisplayPos.x + draw_data->DisplaySize.x) * uiScale;
    float T = draw_data->DisplayPos.y * uiScale;
    float B = (draw_data->DisplayPos.y + draw_data->DisplaySize.y) * uiScale;
    mathfu::mat4 ortho_projection =
        {
            { 2.0f/(R-L),   0.0f,         0.0f,   0.0f },
            { 0.0f,         2.0f/(T-B),   0.0f,   0.0f },
            { 0.0f,         0.0f,        -1.0f,   0.0f },
            { (R+L)/(L-R),  (T+B)/(B-T),  0.0f,   1.0f },
        };

    if (m_device->getIsVulkanAxisSystem()) {
        ortho_projection = MathHelper::getVulkanMat4Fix() * ortho_projection;
    }

    //UBO update start
    auto &uni = m_imguiUbo->getObject();
    uni.projectionMat = ortho_projection;
    uni.scale[0] = uiScale;
    m_imguiUbo->save();
    //UBO update end


    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];

        // Upload vertex/index buffers

        auto vboBuffer = this->createVertexBuffer(cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        auto iboBuffer = this->createIndexBuffer(cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));

        vboBuffer->uploadData(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        iboBuffer->uploadData(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));

        auto vertexBufferBindings = this->createVAO(vboBuffer, iboBuffer);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != NULL)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
//                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
//                    ImGui_ImplOpenGL3_SetupRenderState(draw_data, fb_width, fb_height, vertex_array_object);
//                else
//                    pcmd->UserCallback(cmd_list, pcmd);
                assert(pcmd->UserCallback == NULL);
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

                if (clip_min.x < fb_width && clip_min.y < fb_height && clip_max.x >= 0.0f && clip_max.y >= 0.0f)
                {
                    // Apply scissor/clipping rectangle
                    // Create mesh and add it to collected meshes
                    gMeshTemplate meshTemplate(vertexBufferBindings);
                    meshTemplate.scissorEnabled = true;
                    //Vulkan has different clip offset compared to OGL
                    if (!m_device->getIsVulkanAxisSystem()) {
                        meshTemplate.scissorOffset = {static_cast<int>(clip_min.x * uiScale), static_cast<int>((fb_height - clip_max.y)* uiScale)};
                        meshTemplate.scissorSize = {static_cast<uint32_t>((clip_max.x - clip_min.x) * uiScale), static_cast<uint32_t>((clip_max.y - clip_min.y)* uiScale)};
                    } else {
                        meshTemplate.scissorOffset = {static_cast<int>(clip_min.x * uiScale), static_cast<int>((clip_min.y) * uiScale)};
                        meshTemplate.scissorSize = {static_cast<uint32_t>((clip_max.x - clip_min.x)* uiScale), static_cast<uint32_t>((clip_max.y - clip_min.y)* uiScale)};
                    }
                    if (meshTemplate.scissorOffset[0] < 0) meshTemplate.scissorOffset[0] = 0;
                    if (meshTemplate.scissorOffset[1] < 0) meshTemplate.scissorOffset[1] = 0;
                    if (meshTemplate.scissorSize[0] < 0) meshTemplate.scissorSize[0] = 0;
                    if (meshTemplate.scissorSize[1] < 0) meshTemplate.scissorSize[1] = 0;

                    meshTemplate.start = pcmd->IdxOffset * sizeof(ImDrawIdx);
                    meshTemplate.end = pcmd->ElemCount;

                    auto material = m_materialCacheIdMap[pcmd->TextureId].lock();

                    if (material != nullptr) {
                        meshes.push_back(this->createMesh(meshTemplate, material));
                    }
                }
            }
        }
    }
}

HGSamplableTexture FrontendUIRenderer::uploadFontTexture(unsigned char *pixels, int width, int height) {
    this->fontTexture = m_device->createTexture(false, false);
    this->fontTexture->getTexture()->loadData(width, height, pixels, ITextureFormat::itRGBA);
    return this->fontTexture;
}
