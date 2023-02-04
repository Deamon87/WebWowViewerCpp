//
// Created by Deamon on 12.12.22.
//

#ifndef AWEBWOWVIEWERCPP_FRONTENDUIRENDERFORWARDVLK_H
#define AWEBWOWVIEWERCPP_FRONTENDUIRENDERFORWARDVLK_H


#include <queue>
#include "../FrontendUIRenderer.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/GDeviceVulkan.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/buffers/GBufferVLK.h"
#include "../../../../../wowViewerLib/src/gapi/UniformBufferStructures.h"
#include "../../../../../wowViewerLib/src/gapi/interface/materials/IMaterial.h"

class FrontendUIRenderForwardVLK : public FrontendUIRenderer {
public:
    explicit FrontendUIRenderForwardVLK(const HGDeviceVLK &hDevice);
    ~FrontendUIRenderForwardVLK() override = default;

    void updateAndDraw(const std::shared_ptr<FrameInputParams<ImGuiFramePlan::ImGUIParam>> &frameInputParams, const std::shared_ptr<ImGuiFramePlan::EmptyPlan> &framePlan) override;
public:
    HGVertexBuffer createVertexBuffer(int sizeInBytes) override;
    HGIndexBuffer  createIndexBuffer(int sizeInBytes) override;

    HGVertexBufferBindings createVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) override {
        return nullptr; //VAO doesnt exist in Vulkan
    };
    HGMesh createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) override;
    HMaterial createUIMaterial(const UIMaterialTemplate &materialTemplate) override;


private:
    HGDeviceVLK m_device;

    //Frame counter
    int m_frame = 0;

    //Rendering pipelining
    std::mutex m_inputParamsMtx;
private:
    HGBufferVLK vboBuffer;
    HGBufferVLK iboBuffer;
    HGBufferVLK uboBuffer;

    void createBuffers();

};


#endif //AWEBWOWVIEWERCPP_FRONTENDUIRENDERFORWARDVLK_H
