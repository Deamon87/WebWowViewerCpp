//
// Created by Deamon on 10/5/2023.
//

#ifndef AWEBWOWVIEWERCPP_RENDERVIEWDEFERREDVLK_H
#define AWEBWOWVIEWERCPP_RENDERVIEWDEFERREDVLK_H

#include "../../../../gapi/vulkan/GDeviceVulkan.h"
#include "../../MapSceneParams.h"
#include "../passes/FFXGlowPassVLK.h"

class RenderViewDeferredVLK : public IRenderView {
public:
    RenderViewDeferredVLK(const HGDeviceVLK &device, const HGBufferVLK &uboBuffer, const HGVertexBufferBindings &quadVAO, bool createOutputFBO);
    ~RenderViewDeferredVLK() override = default;

    void update(int width, int height, float glow);

    std::shared_ptr<GRenderPassVLK> getOpaqueRenderPass() {return m_opaqueRenderPass;}
    std::shared_ptr<GRenderPassVLK> getNonOpaqueRenderPass() {return m_nonOpaqueRenderPass;}

    RenderPassHelper beginOpaquePass(CmdBufRecorder &frameBufCmd, bool willExecuteSecondaryBuffs, mathfu::vec4 &clearColor);
    RenderPassHelper beginNonOpaquePass(CmdBufRecorder &frameBufCmd, bool willExecuteSecondaryBuffs, mathfu::vec4 &clearColor);

    void doOpaqueNonOpaqueBarrier(CmdBufRecorder &frameBufCmd);

    void doOutputPass(CmdBufRecorder &frameBufCmd);

    void doPostGlow(CmdBufRecorder &frameBufCmd);
    void doPostFinal(CmdBufRecorder &bufCmd);

    void iterateOverOutputTextures(std::function<void (const std::array<std::shared_ptr<ISamplableTexture>, IDevice::MAX_FRAMES_IN_FLIGHT> &textures, const std::string &name, ITextureFormat textureFormat)> callback) override;
    void readRGBAPixels(int frameNumber, int x, int y, int width, int height, void *outputdata);
private:
    uint32_t m_width = 640;
    uint32_t m_height = 480;

    HGDeviceVLK m_device;
    bool m_createOutputFBO;

    std::shared_ptr<GRenderPassVLK> m_opaqueRenderPass;
    std::shared_ptr<GRenderPassVLK> m_nonOpaqueRenderPass;
    std::array<std::shared_ptr<GFrameBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_colorFrameBuffers;
    std::array<std::shared_ptr<GFrameBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_colorNonOpaqFrameBuffers;
    std::unique_ptr<FFXGlowPassVLK> glowPass;

    std::shared_ptr<GRenderPassVLK> m_outputRenderPass;
    std::array<std::shared_ptr<GFrameBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_outputFrameBuffers;

    void createFrameBuffers();
    std::vector<std::function<void ()>> onUpdates;
};

#endif //AWEBWOWVIEWERCPP_RENDERVIEWDEFERREDVLK_H
