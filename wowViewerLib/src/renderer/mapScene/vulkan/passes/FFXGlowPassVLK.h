//
// Created by Deamon on 3/17/2023.
//

#ifndef AWEBWOWVIEWERCPP_FFXGLOWPASSVLK_H
#define AWEBWOWVIEWERCPP_FFXGLOWPASSVLK_H

#include <memory>

#include "../../MapSceneRenderer.h"
#include "../../../../gapi/vulkan/GDeviceVulkan.h"
#include "../../materials/IMaterialStructs.h"

class FFXGlowPassVLK {
public:

    FFXGlowPassVLK(const HGDeviceVLK &device, const HGBufferVLK &uboBuffer, const HGVertexBufferBindings &quadVAO);
    void assignFFXGlowUBOConsts(float glow);
    void updateDimensions(int width, int height,
                          const std::vector<HGSamplableTexture> &inputColorTextures,
                          const std::shared_ptr<GRenderPassVLK> &finalRenderPass);

    void doPass(CmdBufRecorder &frameBufCmd, CmdBufRecorder &swapChainCmd,
                const std::shared_ptr<GRenderPassVLK> &finalRenderPass,
                ViewPortDimensions &viewPortDimensions);

private:
    static constexpr int GAUSS_PASS_COUNT = 3;
    HGDeviceVLK m_device;

    std::shared_ptr<GRenderPassVLK> m_renderPass;
    HGVertexBufferBindings m_drawQuadVao = nullptr;

    std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> m_ffxGlowVs;
    std::array<std::shared_ptr<IBufferChunk<FXGauss::meshWideBlockPS>>, GAUSS_PASS_COUNT> m_ffxGaussPSs;
    std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> m_ffxGlowPS;

    std::array<std::shared_ptr<GFrameBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_GlowFrameB1;
    std::array<std::shared_ptr<GFrameBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_GlowFrameB2;

    std::array<std::array<std::shared_ptr<IMaterial>, GAUSS_PASS_COUNT>, IDevice::MAX_FRAMES_IN_FLIGHT> ffxGaussMat;
    std::array<std::shared_ptr<IMaterial>, IDevice::MAX_FRAMES_IN_FLIGHT> ffxGlowMat;

    std::shared_ptr<IMaterial> createFFXGaussMat(const std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> &ffxGlowVs,
                                                 const std::shared_ptr<IBufferChunk<FXGauss::meshWideBlockPS>> &ffxGlowPS,
                                                 const HGSamplableTexture &texture,
                                                 const PipelineTemplate &pipelineTemplate,
                                                 const std::shared_ptr<GRenderPassVLK> &targetRenderPass);

    std::shared_ptr<IMaterial> createFFXGlowMat(const std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> &ffxGlowVs,
                                                const std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> &ffxGlowPS,
                                                const HGSamplableTexture &screenTex,
                                                const HGSamplableTexture &blurTex,
                                                const PipelineTemplate &pipelineTemplate,
                                                const std::shared_ptr<GRenderPassVLK> &targetRenderPass);

    void createFrameBuffers(int width, int height);

    std::shared_ptr<GFrameBufferVLK> getTargetFrameBuffer(int GAUSS_PASS_I, int frameInFlightI);
    inline void drawMaterial (CmdBufRecorder& cmdBuffer, const std::shared_ptr<IMaterial> &mat);
};


#endif //AWEBWOWVIEWERCPP_FFXGLOWPASSVLK_H
