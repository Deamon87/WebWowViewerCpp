//
// Created by Deamon on 12.12.22.
//

#include "FrontendUIRenderForwardVLK.h"
#include "../../../../../wowViewerLib/src/gapi/UniformBufferStructures.h"
#include "buffers/IVertexBufferDynamicImguiVLK.h"

FrontendUIRenderForwardVLK::FrontendUIRenderForwardVLK(HGDeviceVLK hDevice) : m_device(hDevice) {
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        createBuffer(vertexBuffers[i], 1024*1024*1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        createBuffer(indexBuffers[i], 1024*1024*1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

void FrontendUIRenderForwardVLK::putIntoQueue(std::shared_ptr<FrontendUIInputParams> &frameInputParams) {
    std::lock_guard<std::mutex> guard(m_inputParamsMtx);

    m_inputParams.push(frameInputParams);
}

void FrontendUIRenderForwardVLK::render() {

}

void FrontendUIRenderForwardVLK::createBuffer(Buffer &buffer, int bufferSize, VkBufferUsageFlags bufferUsageFlags) {
//Create new buffer for VBO
    VkBufferCreateInfo vbInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    vbInfo.size = bufferSize;
    vbInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vbInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo ibAllocCreateInfo = {};
    ibAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    ibAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    ERR_GUARD_VULKAN(vmaCreateBuffer(m_device->getVMAAllocator(), &vbInfo, &ibAllocCreateInfo,
                                     &buffer.stagingBuffer,
                                     &buffer.stagingBufferAlloc,
                                     &buffer.stagingBufferAllocInfo));

    // No need to flush stagingVertexBuffer memory because CPU_ONLY memory is always HOST_COHERENT.
    vbInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferUsageFlags;
    ibAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    ibAllocCreateInfo.flags = 0;
    ERR_GUARD_VULKAN(vmaCreateBuffer(m_device->getVMAAllocator(), &vbInfo, &ibAllocCreateInfo,
                                     &buffer.g_hBuffer,
                                     &buffer.g_hBufferAlloc, nullptr));

    //Create virtual buffer off this native buffer
    VmaVirtualBlockCreateInfo blockCreateInfo = {};
    blockCreateInfo.size = bufferSize;

    VkResult res = vmaCreateVirtualBlock(&blockCreateInfo, &buffer.virtualBlock);
    if(res != VK_SUCCESS)
    {
        std::cerr << "Failed to create virtual buffer" << std::endl;
    }
    //memcpy(bufferInfo.stagingVertexBufferAllocInfo.pMappedData, data, length);
}

void FrontendUIRenderForwardVLK::destroyBuffer(FrontendUIRenderForwardVLK::Buffer &buffer) {
    auto l_device = m_device;

    m_device->addDeallocationRecord(
        [buffer, l_device]() {
            vmaClearVirtualBlock(buffer.virtualBlock);
            vmaDestroyVirtualBlock(buffer.virtualBlock);

            vmaDestroyBuffer(l_device->getVMAAllocator(), buffer.stagingBuffer, buffer.stagingBufferAlloc);
            vmaDestroyBuffer(l_device->getVMAAllocator(), buffer.g_hBuffer, buffer.g_hBufferAlloc);

        }
    );
}

HIVertexBufferDynamicImgui FrontendUIRenderForwardVLK::allocateDynamicVertexBuffer(int maxSize) {
    VmaVirtualAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.size = maxSize; //Size in bytes

    VmaVirtualAllocation alloc;
    VkDeviceSize offset;

    VkResult res = vmaVirtualAllocate(vertexBuffers[0].virtualBlock, &allocCreateInfo, &alloc, &offset);
    if(res == VK_SUCCESS)
    {
        return std::make_shared<RVertexBufferDynamicImguiVLK>(this, maxSize, alloc, offset);
    }
    else
    {
        // Allocation failed - no space for it could be found. Handle this error!
        return nullptr;
    }
}

void FrontendUIRenderForwardVLK::update() {

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

    auto *draw_data = frameParams->additionalData->imData;
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

    if (m_device->getIsVulkanAxisSystem()) {
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

    auto shaderPermute = m_device->getShader("imguiShader", nullptr);
    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];

        // Upload vertex/index buffers
        auto vertexBufferBindings = m_device->createVertexBufferBindings();
        auto vboBuffer = m_device->createVertexBuffer();
        auto iboBuffer = m_device->createIndexBuffer();

        vboBuffer->uploadData(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        iboBuffer->uploadData(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));

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

                    meshTemplate.ubo[1] = uboPart;
                    meshTemplate.textureCount = 1;
                    meshTemplate.texture[0] = pcmd->TextureId;

                    meshTemplate.start = pcmd->IdxOffset * 2;
                    meshTemplate.end = pcmd->ElemCount;

                    resultDrawStage->opaqueMeshes->meshes.push_back(m_device->createMesh(meshTemplate));
                }
            }
        }
    }

//    //1. Collect buffers
//    auto &bufferChunks = updateStages[0]->uniformBufferChunks;
//    int renderIndex = 0;
//    for (const auto &mesh : resultDrawStage->opaqueMeshes->meshes) {
//        for (int i = 0; i < 5; i++ ) {
//            auto bufferChunk = mesh->getUniformBuffer(i);
//
//            if (bufferChunk != nullptr) {
//                bufferChunks.push_back(bufferChunk);
//            }
//        }
//    }
//
//    std::sort( bufferChunks.begin(), bufferChunks.end());
//    bufferChunks.erase( unique( bufferChunks.begin(), bufferChunks.end() ), bufferChunks.end() );
}
