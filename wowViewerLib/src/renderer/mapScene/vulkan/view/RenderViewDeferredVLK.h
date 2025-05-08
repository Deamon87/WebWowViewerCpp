//
// Created by Deamon on 10/5/2023.
//

#ifndef AWEBWOWVIEWERCPP_RENDERVIEWDEFERREDVLK_H
#define AWEBWOWVIEWERCPP_RENDERVIEWDEFERREDVLK_H

#include "../../../../gapi/vulkan/GDeviceVulkan.h"
#include "../../../../gapi/interface/FrameContext.h"
#include "../../MapSceneParams.h"
#include "../passes/FFXGlowPassVLK.h"
#include "VideoRecordingContextVLK.h"
#include "HiZOcclusionCullingVLK.h"
#include "LiquidHiZVLK.h"

class RenderViewDeferredVLK : public IRenderView {
public:
    RenderViewDeferredVLK(const HGDeviceVLK &device, const HGBufferVLK &uboBuffer,
                          const HGBufferVLK &pointLightBuffer,
                          const HGBufferVLK &spotLightBuffer,
                          const std::shared_ptr<GDescriptorSet> &sceneWideDS,
                          const std::shared_ptr<GDescriptorSet> &gBufferDataDS,
                          const HGVertexBufferBindings &quadVAO,
                          const HGVertexBufferBindings &spotVAO,
                          const HGVertexBufferBindings &spotLineVAO,
                          bool createOutputFBO);
    ~RenderViewDeferredVLK() override = default;

    void update(int width, int height, float glow,
                const std::vector<LocalLight> &pointLights,
                const std::vector<SpotLight> &spotLights,
                const std::vector<SpotLight> &insideSpotLights);

    void updateExternalDSes();

    std::shared_ptr<GRenderPassVLK> getGBufferPass() {return m_gBufferRenderPass;}
    std::shared_ptr<GRenderPassVLK> getLiquidDepthPass() {return m_liquidDepthRenderPass;}
    std::shared_ptr<GFrameBufferVLK> getGBufferFrameBuffer() {
        auto frameNum = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;
        return m_gBufferFrameBuffers[frameNum];
    }
    std::shared_ptr<GRenderPassVLK> getForwardPass() {return m_forwardRenderPass;}
    // Load-load variant used by the below/above liquid forward passes; secondary command
    // buffers executed in those passes must inherit from this exact render pass.
    std::shared_ptr<GRenderPassVLK> getForwardPassNoClear() {return m_forwardRenderPassNoClear;}
    // Render area used by the forward passes, for secondary command buffer viewport/scissor setup
    std::array<uint32_t, 2> getForwardRenderAreaSize() const { return {m_width, m_height}; }

    RenderPassHelper beginGBufferPass(CmdBufRecorder &frameBufCmd, bool willExecuteSecondaryBuffs, mathfu::vec4 &clearColor);
    RenderPassHelper beginForwardPass(CmdBufRecorder &frameBufCmd, bool willExecuteSecondaryBuffs, bool clearDepth, mathfu::vec4 &clearColor, bool clearColorBuf = true);

    void doGBufferBarrier(CmdBufRecorder &frameBufCmd);

    // Enable/disable FXAA post-processing (composite → intermediate → FXAA → target)
    void setFxaaEnabled(bool enabled) { m_fxaaEnabled = enabled; }

    // Composite forward color + glow into the FXAA-input intermediate (own render pass)
    void doComposite(CmdBufRecorder &frameBufCmd);
    // Draw FXAA (from the intermediate) into the currently active render pass (e.g. swapchain)
    void doPostFinalFxaa(CmdBufRecorder &bufCmd);

    // Run Hi-Z generation + occlusion culling between GBuffer and forward passes.
    // Call after GBuffer render pass ends, before doGBufferBarrier.
    void doHiZOcclusionCulling(CmdBufRecorder &cmdBuf,
                                const mathfu::mat4 &viewProj,
                                const HGBufferVLK &aabbBuffer,
                                const HGBufferVLK &visibilityBuffer,
                                uint32_t objectCount);

    void doLightPass(CmdBufRecorder &frameBufCmd);
    void doDebugLightPass(CmdBufRecorder &frameBufCmd);
    void doOutputPass(CmdBufRecorder &frameBufCmd);

    void doPostGlow(CmdBufRecorder &frameBufCmd);
    void doPostFinal(CmdBufRecorder &bufCmd);

    // Liquid depth + Hi-Z
    RenderPassHelper beginLiquidDepthPass(CmdBufRecorder &cmd);
    void doLiquidHiZ(CmdBufRecorder &cmdBuf);
    HGSamplableTexture getLiquidDepthTexture();
    LiquidHiZVLK* getLiquidHiZ() { return m_liquidHiZ.get(); }

    // Copy forward color attachment to backbuffer texture for liquid shader sampling
    void copyForwardColorToLiquidBackBuffer(CmdBufRecorder &cmdBuf);

    // Copy a single pixel from the object-id attachment (forward attachment 1) into a host-visible buffer,
    // for async GPU object-id picking. Safe to call only outside an active render pass.
    void copyObjectIdPixelToBuffer(CmdBufRecorder &cmdBuf, int x, int y, VkBuffer dstBuffer);

    void iterateOverOutputTextures(std::function<void (const std::array<std::shared_ptr<ISamplableTexture>, IDevice::MAX_FRAMES_IN_FLIGHT> &textures, const std::string &name, ITextureFormat textureFormat)> callback) override;
    void readRGBAPixels(int frameNumber, int x, int y, int width, int height, void *outputdata) override;
    
    std::shared_ptr<IVideoRecordingContext> createVideoRecordingContext(uint32_t framebufferWidth, uint32_t framebufferHeight, uint32_t outputWidth, uint32_t outputHeight, const std::string &outputFilename) override;
    void feedFrameToVideoRecording(std::shared_ptr<IVideoRecordingContext> context, int frameNumber) override;
private:
    uint32_t m_width = 1;
    uint32_t m_height = 1;

    std::array<uint32_t,IDevice::MAX_FRAMES_IN_FLIGHT> m_pointLightCounts;
    std::array<uint32_t,IDevice::MAX_FRAMES_IN_FLIGHT> m_spotLightCounts;
    std::array<uint32_t,IDevice::MAX_FRAMES_IN_FLIGHT> m_insideSpotLightCounts;

    std::shared_ptr<GDescriptorSet> m_sceneWideDS;
    std::shared_ptr<GDescriptorSet> m_gBufferDataDS;

    HGDeviceVLK m_device;

    bool m_createOutputFBO;
    bool m_lightMatsCreated = false;

    HGVertexBufferBindings m_quadVAO;
    HGVertexBufferBindings m_spotVAO;
    HGVertexBufferBindings m_spotLineVAO;

    HGBufferVLK m_pointLightDataBuffer;
    HGBufferVLK m_spotLightDataBuffer;
    std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> m_lightScreenSize;

    std::shared_ptr<GRenderPassVLK> m_gBufferRenderPass;
    std::shared_ptr<GRenderPassVLK> m_forwardRenderPass;
    std::shared_ptr<GRenderPassVLK> m_forwardRenderPassNoDepthClear;
    std::shared_ptr<GRenderPassVLK> m_forwardRenderPassNoClear;
    std::shared_ptr<GRenderPassVLK> m_lightBufferPass;
    std::shared_ptr<GRenderPassVLK> m_shadowPass;
    std::array<std::shared_ptr<GFrameBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_gBufferFrameBuffers;
    std::array<std::shared_ptr<GFrameBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_forwardFrameBuffers;
    std::array<std::shared_ptr<GFrameBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_lightFrameBuffers;


    std::array<std::shared_ptr<IBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_pointLightBuffers;
    std::array<std::shared_ptr<IBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_spotLightBuffers;
    std::array<std::shared_ptr<IBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_insideSpotLightBuffers;
    std::array<std::shared_ptr<ISimpleMaterialVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_pointLightMats;
    std::array<std::shared_ptr<ISimpleMaterialVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_spotLightMats;
    std::array<std::shared_ptr<ISimpleMaterialVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_insideSpotLightMats;
    std::array<std::shared_ptr<ISimpleMaterialVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_spotLightDebugMats;


    std::unique_ptr<FFXGlowPassVLK> glowPass;
    std::unique_ptr<HiZOcclusionCullingVLK> m_hiZOcclusionCulling;

    // Liquid depth buffer and Hi-Z
    std::shared_ptr<GRenderPassVLK> m_liquidDepthRenderPass;
    std::array<std::shared_ptr<GFrameBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_liquidDepthFrameBuffers;
    std::unique_ptr<LiquidHiZVLK> m_liquidHiZ;

    // Liquid backbuffer: copy of forward color for liquid shader refraction sampling
    std::array<std::shared_ptr<GTextureVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_liquidBackBufferTextures;
    std::array<HGSamplableTexture, IDevice::MAX_FRAMES_IN_FLIGHT> m_liquidBackBufferSamplable;

    std::shared_ptr<GRenderPassVLK> m_outputRenderPass;
    std::array<std::shared_ptr<GFrameBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_outputFrameBuffers;

    // FXAA: composite (forward color + glow) renders into this intermediate,
    // then an FXAA fullscreen pass turns it into the final image
    std::shared_ptr<GRenderPassVLK> m_fxaaInputRenderPass;
    std::array<std::shared_ptr<GFrameBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_fxaaInputFrameBuffers;
    std::array<std::shared_ptr<ISimpleMaterialVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_fxaaMats;
    std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> m_fxaaVsParams;
    std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> m_fxaaScreenSize;
    bool m_fxaaEnabled = false;
    bool m_fxaaMatsBuilt = false;
    bool m_lastGlowTargetIsFxaa = false;

    void createFrameBuffers();
    void createLightBufferMats();
    void createFxaaMats();
    void drawFxaaQuad(CmdBufRecorder &cmdBuf, int frameNum);
    std::vector<std::function<void ()>> onUpdates;

    void updateLightBuffers(const std::vector<LocalLight> &pointLights, const std::vector<SpotLight> &spotLights, const std::vector<SpotLight> &insideSpotLights);

    // Generic barrier function that works for both color and depth textures
    // forAttachmentRead: true = transition for attachment read, false = transition for shader sampling
    void addAttachmentToShaderReadBarrier(CmdBufRecorder &cmdBuf, const HGSamplableTexture &texture, bool forAttachmentRead = false);

    // Legacy function for backward compatibility
    void addColorAttachmentToShaderReadBarrier(CmdBufRecorder &cmdBuf, const HGSamplableTexture &texture);
};

#endif //AWEBWOWVIEWERCPP_RENDERVIEWDEFERREDVLK_H
