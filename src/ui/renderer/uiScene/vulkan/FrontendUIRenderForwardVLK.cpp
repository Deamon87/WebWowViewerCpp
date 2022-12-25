//
// Created by Deamon on 12.12.22.
//

#include "FrontendUIRenderForwardVLK.h"
#include "../../../../../wowViewerLib/src/gapi/UniformBufferStructures.h"

FrontendUIRenderForwardVLK::FrontendUIRenderForwardVLK(HGDeviceVLK hDevice) : m_device(hDevice) {
}

void FrontendUIRenderForwardVLK::putIntoQueue(std::shared_ptr<FrontendUIInputParams> &frameInputParams) {
    std::lock_guard<std::mutex> guard(m_inputParamsMtx);

    m_inputParams.push(frameInputParams);
}

void FrontendUIRenderForwardVLK::update(VkCommandBuffer updateBuffer, VkCommandBuffer swapChainDraw) {

    std::unique_lock<std::mutex> guard(m_inputParamsMtx);

    guard.lock();
    auto frameParams = m_inputParams.front();
    m_inputParams.pop();
    guard.unlock();

//    if (this->fontTexture == nullptr) {
//        ImGuiIO& io = ImGui::GetIO();
//        unsigned char* pixels;
//        int width, height;
//        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.
//        // Upload texture to graphics system
//        this->fontTexture = m_device->createTexture(false, false);
//        this->fontTexture->loadData(width, height, pixels, ITextureFormat::itRGBA);
//        // Store our identifier
//        io.Fonts->TexID = this->fontTexture;
//        return;
//    }
//    if (exporter != nullptr) {
//        if (m_processor->completedAllJobs() && !m_api->hDevice->wasTexturesUploaded()) {
//            exporterFramesReady++;
//        }
//        if (exporterFramesReady > 5) {
//            exporter->saveToFile("model.gltf");
//            exporter = nullptr;
//        }
//    }
    int lastWidth = frameParams->viewPortDimensions.maxs[0];
    int lastHeight = frameParams->viewPortDimensions.maxs[1];

    ImDrawData *draw_data = nullptr; //frameParams->additionalData->imData;
    if (draw_data == nullptr)
        return;

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

    //Apply vulkan Matrix fix
    if (true) {
        static const mathfu::mat4 vulkanMatrixFix1 = mathfu::mat4(1, 0, 0, 0,
                                                                 0, -1, 0, 0,
                                                                 0, 0, 1.0/2.0, 1/2.0,
                                                                 0, 0, 0, 1).Transpose();
        ortho_projection = vulkanMatrixFix1 * ortho_projection;
    }

    auto uboPart = m_device->createUniformBufferChunk(sizeof(ImgUI::modelWideBlockVS));


    uboPart->setUpdateHandler([ortho_projection,uiScale](IUniformBufferChunk* self, const HFrameDependantData &frameDepedantData) {
        auto &uni = self->getObject<ImgUI::modelWideBlockVS>();
        uni.projectionMat = ortho_projection;
        uni.scale[0] = uiScale;
    });

    auto shaderPermute = m_device->getShader("imguiShader", "imguiShader", nullptr);

    //Calc total size of index/vertex buffers needed
    int vertexBufferSize = 0;
    int indexBufferSize = 0;
    for (int i = 0; i < draw_data->CmdListsCount; i++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[i];
        vertexBufferSize += cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
        indexBufferSize += cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);
    }

    auto vboSubBuf = vboBuffer->getSubBuffer(vertexBufferSize);
    auto iboSubBuf = iboBuffer->getSubBuffer(indexBufferSize);

    int vboOffset = 0;
    int iboOffset = 0;
    for (int i = 0; i < draw_data->CmdListsCount; i++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[i];

        vboSubBuf->subUploadData(cmd_list->VtxBuffer.Data, vboOffset, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        iboSubBuf->subUploadData(cmd_list->IdxBuffer.Data, iboOffset, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));

        vboOffset += cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
        iboOffset += cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);
    }




    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];

        // Upload vertex/index buffers
        auto vertexBufferBindings = m_device->createVertexBufferBindings();

        //Create vao
        GVertexBufferBinding vertexBufferBinding;
        vertexBufferBinding.bindings = std::vector<GBufferBinding>(&imguiBindings[0], &imguiBindings[3]);
        vertexBufferBinding.vertexBuffer = vboBuffer;

        vertexBufferBindings->setIndexBuffer(iboBuffer);
        vertexBufferBindings->addVertexBufferBinding(vertexBufferBinding);
        vertexBufferBindings->save();

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
                ImVec4 clip_rect;
                clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
                clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
                clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
                clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

                if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
                {
                    // Apply scissor/clipping rectangle
                    // Create mesh, add it to collected meshes
                    gMeshTemplate meshTemplate(vertexBufferBindings, shaderPermute);
                    meshTemplate.element = DrawElementMode::TRIANGLES;
                    meshTemplate.blendMode = EGxBlendEnum::GxBlend_Alpha;
                    meshTemplate.backFaceCulling = false;
                    meshTemplate.depthCulling = false;

                    meshTemplate.scissorEnabled = true;
                    //Vulkan has different clip offset compared to OGL
                    if (!m_device->getIsVulkanAxisSystem()) {
                        meshTemplate.scissorOffset = {(int)(clip_rect.x* uiScale), (int)((fb_height - clip_rect.w)* uiScale)};
                        meshTemplate.scissorSize = {(int)((clip_rect.z - clip_rect.x) * uiScale), (int)((clip_rect.w - clip_rect.y)* uiScale)};
                    } else {
                        meshTemplate.scissorOffset = {(int)(clip_rect.x * uiScale), (int)((clip_rect.y) * uiScale)};
                        meshTemplate.scissorSize = {(int)((clip_rect.z - clip_rect.x)* uiScale), (int)((clip_rect.w - clip_rect.y)* uiScale)};
                    }

//                    meshTemplate.ubo[1] = uboPart;
//                    meshTemplate.textureCount = 1;
//                    meshTemplate.texture[0] = pcmd->TextureId;
//
//                    meshTemplate.start = pcmd->IdxOffset * 2;
//                    meshTemplate.end = pcmd->ElemCount;
//
//                    resultDrawStage->opaqueMeshes->meshes.push_back(m_device->createMesh(meshTemplate));
                }
            }
        }
    }
}

void FrontendUIRenderForwardVLK::createBuffers() {
    vboBuffer = m_device->createIndexBuffer(1024*1024);
    iboBuffer = m_device->createVertexBuffer(1024*1024);
    uboBuffer = m_device->createUniformBuffer(sizeof(ImgUI::modelWideBlockVS)*IDevice::MAX_FRAMES_IN_FLIGHT);
}
