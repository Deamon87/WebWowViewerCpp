//
// Created by Deamon on 10/5/2023.
//

#ifndef AWEBWOWVIEWERCPP_RENDERVIEWFORWARDVLK_H
#define AWEBWOWVIEWERCPP_RENDERVIEWFORWARDVLK_H

#include "../../../../gapi/vulkan/GDeviceVulkan.h"
#include "../../MapSceneParams.h"
#include "../passes/FFXGlowPassVLK.h"

class RenderViewForwardVLK : public IRenderView {
public:
    RenderViewForwardVLK(const HGDeviceVLK &device, const HGBufferVLK &uboBuffer, const HGVertexBufferBindings &quadVAO, bool createOutputFBO);
    ~RenderViewForwardVLK() override = default;

    void update(int width, int height, float glow);

    RenderPassHelper beginPass(CmdBufRecorder &frameBufCmd, const std::shared_ptr<GRenderPassVLK> &renderPass,
                               bool willExecuteSecondaryBuffs, mathfu::vec4 &clearColor);

    void doOutputPass(CmdBufRecorder &frameBufCmd);

    void doPostGlow(CmdBufRecorder &frameBufCmd);
    void doPostFinal(CmdBufRecorder &bufCmd);

    void iterateOverOutputTextures(std::function<void (const std::array<std::shared_ptr<ISamplableTexture>, IDevice::MAX_FRAMES_IN_FLIGHT> &textures, const std::string &name, ITextureFormat textureFormat)> callback) override;
    void readRGBAPixels(int frameNumber, int x, int y, int width, int height, void *outputdata) override;

    std::shared_ptr<GRenderPassVLK> getRenderPass() {return m_mainRenderPass;}
private:
    uint32_t m_width = 640;
    uint32_t m_height = 480;

    HGDeviceVLK m_device;
    bool m_createOutputFBO;

    std::shared_ptr<GRenderPassVLK> m_mainRenderPass;
    std::array<std::shared_ptr<GFrameBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_colorFrameBuffers;
    std::unique_ptr<FFXGlowPassVLK> glowPass;

    std::shared_ptr<GRenderPassVLK> m_outputRenderPass;
    std::array<std::shared_ptr<GFrameBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_outputFrameBuffers;

    void createFrameBuffers();
    std::vector<std::function<void ()>> onUpdates;
};

#endif //AWEBWOWVIEWERCPP_RENDERVIEWFORWARDVLK_H
