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
    RenderViewDeferredVLK(const HGDeviceVLK &device, const HGBufferVLK &uboBuffer,
                          const HGBufferVLK &pointLightBuffer,
                          const HGBufferVLK &spotLightBuffer,
                          const std::shared_ptr<GDescriptorSet> &sceneWideDS,
                          const HGVertexBufferBindings &quadVAO,
                          const HGVertexBufferBindings &spotVAO,
                          bool createOutputFBO);
    ~RenderViewDeferredVLK() override = default;

    void update(int width, int height, float glow,
                const std::vector<LocalLight> &pointLights,
                const std::vector<SpotLight> &spotLights);

    void setLightBuffers(const std::shared_ptr<GDescriptorSet> &sceneWideDS);

    std::shared_ptr<GRenderPassVLK> getGBufferPass() {return m_gBufferRenderPass;}
    std::shared_ptr<GRenderPassVLK> getForwardPass() {return m_forwardRenderPass;}

    RenderPassHelper beginGBufferPass(CmdBufRecorder &frameBufCmd, bool willExecuteSecondaryBuffs, mathfu::vec4 &clearColor);
    RenderPassHelper beginForwardPass(CmdBufRecorder &frameBufCmd, bool willExecuteSecondaryBuffs, bool clearDepth, mathfu::vec4 &clearColor);

    void doGBufferBarrier(CmdBufRecorder &frameBufCmd);
    void doLightPass(CmdBufRecorder &frameBufCmd);
    void doOutputPass(CmdBufRecorder &frameBufCmd);

    void doPostGlow(CmdBufRecorder &frameBufCmd);
    void doPostFinal(CmdBufRecorder &bufCmd);

    void iterateOverOutputTextures(std::function<void (const std::array<std::shared_ptr<ISamplableTexture>, IDevice::MAX_FRAMES_IN_FLIGHT> &textures, const std::string &name, ITextureFormat textureFormat)> callback) override;
    void readRGBAPixels(int frameNumber, int x, int y, int width, int height, void *outputdata) override;
private:
    uint32_t m_width = 0;
    uint32_t m_height = 0;

    std::array<uint32_t,IDevice::MAX_FRAMES_IN_FLIGHT> m_pointLightCounts;
    std::array<uint32_t,IDevice::MAX_FRAMES_IN_FLIGHT> m_spotLightCounts;

    std::shared_ptr<GDescriptorSet> m_sceneWideDS;

    HGDeviceVLK m_device;

    bool m_createOutputFBO;
    bool m_lightMatsCreated = false;

    HGVertexBufferBindings m_quadVAO;
    HGVertexBufferBindings m_spotVAO;

    HGBufferVLK m_pointLightDataBuffer;
    HGBufferVLK m_spotLightDataBuffer;
    std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> m_lightScreenSize;

    std::shared_ptr<GRenderPassVLK> m_gBufferRenderPass;
    std::shared_ptr<GRenderPassVLK> m_forwardRenderPass;
    std::shared_ptr<GRenderPassVLK> m_forwardRenderPassNoDepthClear;
    std::shared_ptr<GRenderPassVLK> m_lightBufferPass;
    std::shared_ptr<GRenderPassVLK> m_shadowPass;
    std::array<std::shared_ptr<GFrameBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_gBufferFrameBuffers;
    std::array<std::shared_ptr<GFrameBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_forwardFrameBuffers;
    std::array<std::shared_ptr<GFrameBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_lightFrameBuffers;


    std::array<std::shared_ptr<IBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_pointLightBuffers;
    std::array<std::shared_ptr<IBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_spotLightBuffers;
    std::array<std::shared_ptr<ISimpleMaterialVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_pointLightMats;
    std::array<std::shared_ptr<ISimpleMaterialVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_spotLightMats;


    std::unique_ptr<FFXGlowPassVLK> glowPass;

    std::shared_ptr<GRenderPassVLK> m_outputRenderPass;
    std::array<std::shared_ptr<GFrameBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_outputFrameBuffers;

    void createFrameBuffers();
    void createLightBufferMats();
    std::vector<std::function<void ()>> onUpdates;

    void updateLightBuffers(const std::vector<LocalLight> &pointLights, const std::vector<SpotLight> &spotLights);
};

#endif //AWEBWOWVIEWERCPP_RENDERVIEWDEFERREDVLK_H
