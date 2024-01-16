//
// Created by Deamon on 12.12.22.
//

#ifndef AWEBWOWVIEWERCPP_FRONTENDUIRENDERFORWARDVLK_H
#define AWEBWOWVIEWERCPP_FRONTENDUIRENDERFORWARDVLK_H


#include <queue>
#include <random>
#include "../FrontendUIRenderer.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/GDeviceVulkan.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/buffers/GBufferVLK.h"
#include "../../../../../wowViewerLib/src/gapi/UniformBufferStructures.h"
#include "../../../../../wowViewerLib/src/gapi/interface/materials/IMaterial.h"
#include "../../../../../wowViewerLib/src/renderer/vulkan/IRenderFunctionVLK.h"
#include "../../../../../wowViewerLib/src/engine/objects/scenes/EntityActorsFactory.h"

class FrontendUIRenderForwardVLK : public FrontendUIRenderer {
public:
    explicit FrontendUIRenderForwardVLK(const HGDeviceVLK &hDevice);
    ~FrontendUIRenderForwardVLK() override = default;

    std::unique_ptr<IRenderFunction> update(const std::shared_ptr<FrameInputParams<ImGuiFramePlan::ImGUIParam>> &frameInputParams, const std::shared_ptr<ImGuiFramePlan::EmptyPlan> &framePlan) override;
public:
    HGVertexBuffer createVertexBuffer(int sizeInBytes) override;
    HGIndexBuffer  createIndexBuffer(int sizeInBytes) override;

    HGVertexBufferBindings createVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) override;;
    HGMesh createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) override;
    std::shared_ptr<IUIMaterial> createUIMaterial(const HGSamplableTexture &hgtexture, bool opaque = false) override;

    uint32_t generateUniqueMatId();
private:
    HGDeviceVLK m_device;

    std::shared_ptr<EntityFactory<GMeshVLK>> meshFactory = std::make_shared<EntityFactory<GMeshVLK>>();

    std::mt19937_64 eng; //Use the 64-bit Mersenne Twister 19937 generator
    //and seed it with entropy.
    std::uniform_int_distribution<unsigned long long> idDistr;

    //Rendering pipelining
    std::mutex m_inputParamsMtx;
private:
    HGBufferVLK vboBuffer;
    HGBufferVLK iboBuffer;
    HGBufferVLK uboBuffer;

    void createBuffers();

    std::shared_ptr<GRenderPassVLK> m_lastRenderPass;
    std::shared_ptr<std::vector<HGMesh>> m_previousMeshes = nullptr;

    HGVertexBufferBindings m_emptyImguiVAO = nullptr;
};


#endif //AWEBWOWVIEWERCPP_FRONTENDUIRENDERFORWARDVLK_H
