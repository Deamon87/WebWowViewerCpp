//
// Created by Deamon on 10/5/2023.
//

#include "RenderViewDeferredVLK.h"
#include "../../../../gapi/vulkan/materials/MaterialBuilderVLK.h"
#include "../../../../gapi/vulkan/buffers/CBufferChunkVLK.h"
#include "VideoRecordingContextVLK.h"
#include "../../../../gapi/vulkan/textures/GTextureVLK.h"
#include "../../../../gapi/vulkan/GVertexBufferBindingsVLK.h"
#include "../../../../gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferRecorder_inline.h"
#include "../../../../gapi/interface/FrameContext.h"

/*
 * RenderViewDeferredVLK
 */

RenderViewDeferredVLK::RenderViewDeferredVLK(const HGDeviceVLK &device,
                                             const HGBufferVLK &uboBuffer,
                                             const HGBufferVLK &pointLightBuffer,
                                             const HGBufferVLK &spotLightBuffer,
                                             const std::shared_ptr<GDescriptorSet> &sceneWideDS,
                                             const std::shared_ptr<GDescriptorSet> &gBufferDataDS,
                                             const HGVertexBufferBindings &quadVAO,
                                             const HGVertexBufferBindings &spotVAO,
                                             const HGVertexBufferBindings &spotLineVAO,
                                             bool createOutputFBO) : m_device(device),
                                             m_quadVAO(quadVAO),
                                             m_spotVAO(spotVAO),
                                             m_spotLineVAO(spotLineVAO),
                                             m_pointLightDataBuffer(pointLightBuffer),
                                             m_spotLightDataBuffer(spotLightBuffer),
                                             m_sceneWideDS(sceneWideDS),
                                             m_gBufferDataDS(gBufferDataDS),
                                             m_createOutputFBO(createOutputFBO) {
    glowPass = std::make_unique<FFXGlowPassVLK>(m_device, uboBuffer, quadVAO);
    m_hiZOcclusionCulling = std::make_unique<HiZOcclusionCullingVLK>(m_device);
    m_liquidHiZ = std::make_unique<LiquidHiZVLK>(m_device);

    for (auto &pointLightBuff : m_pointLightBuffers) {
        pointLightBuff = m_pointLightDataBuffer->getSubBuffer(100*sizeof(LocalLight));
    }
    for (auto &spotLightBuff : m_spotLightBuffers) {
        spotLightBuff = m_spotLightDataBuffer->getSubBuffer(100*sizeof(SpotLight));
    }
    for (auto &spotLightBuff : m_insideSpotLightBuffers) {
        spotLightBuff = m_spotLightDataBuffer->getSubBuffer(3*sizeof(SpotLight));
    }

    createFrameBuffers();

    m_lightScreenSize = std::make_shared<CBufferChunkVLK<mathfu::vec4_packed>>(uboBuffer);

    // NOTE: the *values* are written in update(), not here. Writes made at ctor time go through
    // the staging ring buffer with a stale frame-in-flight index, so they may never be copied
    // to the GPU before the first draw (this caused the degenerate-quad / black-screenshot bug).
    m_fxaaVsParams = std::make_shared<CBufferChunkVLK<mathfu::vec4_packed>>(uboBuffer);
    m_fxaaScreenSize = std::make_shared<CBufferChunkVLK<mathfu::vec4_packed>>(uboBuffer);
}

void RenderViewDeferredVLK::createFrameBuffers() {
    {
        auto const gBufferFormat = {
            ITextureFormat::itRGBA,        //Normals
            ITextureFormat::itFloat32,        //Depth packed Readable
        };

        // The object pick id attachment (R32_UINT, non-blended) requires the independentBlend
        // feature for per-attachment blend state. Without it, fall back to a single color
        // attachment; shaders then skip the outObjectId write via specialization constant.
        bool selectionSupported = m_device->supportsSelection();
        auto const forwardBufferFormat = selectionSupported ?
            std::vector<ITextureFormat>{ITextureFormat::itRGBA, ITextureFormat::itInt} :
            std::vector<ITextureFormat>{ITextureFormat::itRGBA};

        auto const lightBufferFormat = {
            ITextureFormat::itRGBA16   //Color
        };


        auto depthFormat = ITextureFormat::itDepth32;
        bool invertZ = false;

        int gBufferPassSamples = 1;
        int forwardPassSamples = 1; //m_device->getMaxSamplesCnt();

        m_gBufferRenderPass = m_device->getRenderPass(gBufferFormat,
                                                     depthFormat,
                                                     sampleCountToVkSampleCountFlagBits(gBufferPassSamples),
                                                     invertZ, false,
                                                     true, true);

        m_forwardRenderPass = m_device->getRenderPass(forwardBufferFormat,
                                                        depthFormat,
                                                        sampleCountToVkSampleCountFlagBits(forwardPassSamples),
                                                        invertZ, false,
                                                        true, true);

        m_forwardRenderPassNoDepthClear = m_device->getRenderPass(forwardBufferFormat,
                                                        depthFormat,
                                                        sampleCountToVkSampleCountFlagBits(forwardPassSamples),
                                                        invertZ, false,
                                                        true, false);

        m_forwardRenderPassNoClear = m_device->getRenderPass(forwardBufferFormat,
                                                        depthFormat,
                                                        sampleCountToVkSampleCountFlagBits(forwardPassSamples),
                                                        invertZ, false,
                                                        false, false);

        m_lightBufferPass = m_device->getRenderPass(lightBufferFormat,
                                                    depthFormat,
                                                    sampleCountToVkSampleCountFlagBits(gBufferPassSamples),
                                                    invertZ, false,
                                                    true, false);

        for (auto &gBufferFrameBuffer: m_gBufferFrameBuffers) {
            gBufferFrameBuffer = std::make_shared<GFrameBufferVLK>(
                *m_device,
                m_gBufferRenderPass,
                nullptr,
                m_width, m_height
            );
        }

//        std::vector<uint8_t> attachmentsToCopy = {};
        for (int i = 0; i < m_forwardFrameBuffers.size(); i++) {
            m_forwardFrameBuffers[i] = std::make_shared<GFrameBufferVLK>(
                *m_device,
                m_forwardRenderPass,
                m_gBufferFrameBuffers[i]->getDepthTexture(),
                m_width,
                m_height
            );
        }
        for (int i = 0; i < m_lightFrameBuffers.size(); i++) {
            m_lightFrameBuffers[i] = std::make_shared<GFrameBufferVLK>(
                *m_device,
                m_lightBufferPass,
                m_gBufferFrameBuffers[i]->getDepthTexture(),
                m_width,
                m_height
            );
        }

        // Liquid depth: depth-only render pass (no color attachments)
        auto const emptyColorFormat = std::initializer_list<ITextureFormat>{};
        m_liquidDepthRenderPass = m_device->getRenderPass(emptyColorFormat,
                                                          depthFormat,
                                                          sampleCountToVkSampleCountFlagBits(forwardPassSamples),
                                                          invertZ, false,
                                                          true, true);

        for (auto &liquidDepthFB : m_liquidDepthFrameBuffers) {
            liquidDepthFB = std::make_shared<GFrameBufferVLK>(
                *m_device,
                m_liquidDepthRenderPass,
                nullptr,  // own depth texture
                m_width, m_height
            );
        }

        // Liquid backbuffer: copy of forward color for liquid refraction
        auto linearSampler = m_device->getSampler(true, false, false);
        for (int i = 0; i < IDevice::MAX_FRAMES_IN_FLIGHT; i++) {
            m_liquidBackBufferTextures[i] = std::make_shared<GTextureVLK>(
                *m_device,
                m_width, m_height,
                false,
                VK_FORMAT_R8G8B8A8_UNORM,
                VK_SAMPLE_COUNT_1_BIT,
                1,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
            );
            m_liquidBackBufferSamplable[i] = std::make_shared<ISamplableTexture>(
                m_liquidBackBufferTextures[i], linearSampler);
        }
    }
    if (m_createOutputFBO) {
        auto const dataFormat = {ITextureFormat::itRGBA};
        bool invertZ = false;

        m_outputRenderPass = m_device->getRenderPass(dataFormat, ITextureFormat::itNone,
                                               VK_SAMPLE_COUNT_1_BIT,
                                               invertZ, false, true, true);

        for (auto &outputFrameBuffer: m_outputFrameBuffers) {
            outputFrameBuffer = std::make_shared<GFrameBufferVLK>(
                *m_device,
                m_outputRenderPass,
                nullptr,
                m_width, m_height
            );
        }
    }

    // FXAA intermediate: composite (forward color + glow) renders here when FXAA is on,
    // then the FXAA pass turns it into the final image
    {
        auto const dataFormat = {ITextureFormat::itRGBA};
        bool invertZ = false;

        m_fxaaInputRenderPass = m_device->getRenderPass(dataFormat, ITextureFormat::itNone,
                                                        VK_SAMPLE_COUNT_1_BIT,
                                                        invertZ, false, true, true);

        for (auto &fxaaInputFrameBuffer: m_fxaaInputFrameBuffers) {
            fxaaInputFrameBuffer = std::make_shared<GFrameBufferVLK>(
                *m_device,
                m_fxaaInputRenderPass,
                nullptr,
                m_width, m_height
            );
        }
    }

    // Intermediate textures were recreated — FXAA materials referencing them must be rebuilt
    m_fxaaMatsBuilt = false;
}

static const PipelineTemplate s_lightBufferPipelineT = {
    DrawElementMode::TRIANGLES,
    false,
    true,
    EGxBlendEnum::GxBlend_Add,
    false,
    false,
    0xFF
};

static const PipelineTemplate s_lightBufferPipelineSpot = {
    DrawElementMode::TRIANGLES,
    true,
    true,
    EGxBlendEnum::GxBlend_Add,
    false,
    false,
    0xFF
};

static const PipelineTemplate s_spotLightDebug = {
    DrawElementMode::LINE,
    false,
    true,
    EGxBlendEnum::GxBlend_Opaque,
    false,
    false,
    0xFF
};

void RenderViewDeferredVLK::createLightBufferMats() {
    for (int i = 0; i < m_pointLightMats.size(); i++) {
        m_pointLightMats[i] = MaterialBuilderVLK::fromShader(m_device, {"pointLight", "pointLight"}, {
            .vertexShaderFolder = "bindless/lights",
            .fragmentShaderFolder = "bindless/lights",
            .typeOverrides = {
                {0, {
                    {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
                }}
            }
        })
        .createPipeline(m_quadVAO, m_lightBufferPass, s_lightBufferPipelineT)
        .bindDescriptorSet(0, m_sceneWideDS)
        .bindDescriptorSet(1, m_gBufferDataDS)
        .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(0, m_pointLightBuffers[i])
                .delayUpdate();
        })
        .toMaterial();
    }
    for (int i = 0; i < m_spotLightMats.size(); i++) {
        m_spotLightMats[i] = MaterialBuilderVLK::fromShader(m_device, {"spotLight", "spotLight"}, {
            .vertexShaderFolder = "bindless/lights",
            .fragmentShaderFolder = "bindless/lights",
            .typeOverrides = {
                {0, {
                    {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
                }}
            }
        })
        .createPipeline(m_quadVAO, m_lightBufferPass, s_lightBufferPipelineSpot)
        .bindDescriptorSet(0, m_sceneWideDS)
        .bindDescriptorSet(1, m_gBufferDataDS)
        .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(0, m_spotLightBuffers[i])
                .delayUpdate();
        })
        .toMaterial();
    }
    for (int i = 0; i < m_insideSpotLightMats.size(); i++) {
        m_insideSpotLightMats[i] = MaterialBuilderVLK::fromShader(m_device, {"spotLightFullScreen", "spotLight"}, {
            .vertexShaderFolder = "bindless/lights",
            .fragmentShaderFolder = "bindless/lights",
            .typeOverrides = {
                {0, {
                    {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
                }}
            }
        })
        .createPipeline(m_quadVAO, m_lightBufferPass, s_lightBufferPipelineSpot)
        .bindDescriptorSet(0, m_sceneWideDS)
        .bindDescriptorSet(1, m_gBufferDataDS)
        .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(0, m_insideSpotLightBuffers[i])
                .delayUpdate();
        })
        .toMaterial();
    }

    for (int i = 0; i < m_spotLightDebugMats.size(); i++) {
        m_spotLightDebugMats[i] = MaterialBuilderVLK::fromShader(m_device, {"spotLight", "lightDebugDraw"}, {
                .vertexShaderFolder = "bindless/lights",
                .fragmentShaderFolder = "bindless/lights",
                .typeOverrides = {
                    {0, {
                        {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
                    }}
                }
            })
            .createPipeline(m_spotLineVAO, m_forwardRenderPass, s_spotLightDebug)
            .bindDescriptorSet(0, m_sceneWideDS)
            .bindDescriptorSet(1, m_gBufferDataDS)
            .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
                    ds->beginUpdate()
                        .ssbo(0, m_spotLightBuffers[i])
                        .delayUpdate();
            })
            .toMaterial();
    }


    m_lightMatsCreated = true;
}

void RenderViewDeferredVLK::update(int width, int height, float glow,
                                   const std::vector<LocalLight> &pointLights,
                                   const std::vector<SpotLight> &spotLights,
                                   const std::vector<SpotLight> &insideSpotLights
                                   ) {
    width = std::max<int>(1, width);
    height = std::max<int>(1, height);

    if (width != m_width || height != m_height) {
        m_width = std::max<int>(1, width);
        m_height = std::max<int>(1, height);

        {
            auto &screensize = m_lightScreenSize->getObject();
            screensize = {(float)m_width, (float)m_height, 0.0, 0.0};
            m_lightScreenSize->save();
        }
        // FXAA fullscreen-quad params: {w, h, x, y} viewport scale/offset (full view) and
        // {width, height, 1/width, 1/height} for the shader.
        {
            auto &fxaaVsParams = m_fxaaVsParams->getObject();
            fxaaVsParams = {1.0f, 1.0f, 0.0f, 0.0f};
            m_fxaaVsParams->save();
        }
        {
            auto &fxaaScreenSize = m_fxaaScreenSize->getObject();
            fxaaScreenSize = {(float)m_width, (float)m_height, 1.0f/(float)m_width, 1.0f/(float)m_height};
            m_fxaaScreenSize->save();
        }

        this->createFrameBuffers();
        this->createLightBufferMats();

        {
            std::vector<std::shared_ptr<ISamplableTexture>> inputColorTextures;
            for (int i = 0; i < m_forwardFrameBuffers.size(); i++) {
                inputColorTextures.emplace_back(m_forwardFrameBuffers[i]->getAttachment(0));
            }

            glowPass->updateDimensions(m_width, m_height,
                                       inputColorTextures,
                                       m_fxaaEnabled ? m_fxaaInputRenderPass :
                                           (!this->m_createOutputFBO ? m_device->getSwapChainRenderPass() : this->m_outputRenderPass));
            m_lastGlowTargetIsFxaa = m_fxaaEnabled;
        }

        m_hiZOcclusionCulling->setup(m_width, m_height);
        m_liquidHiZ->setup(m_width, m_height);

        this->executeOnChange();
    }
    if (!m_lightMatsCreated) {
        this->createLightBufferMats();
    }

    // FXAA toggled at runtime: rebuild the glow composite materials so they target
    // the right pass (FXAA intermediate vs swapchain/output)
    if (m_lastGlowTargetIsFxaa != m_fxaaEnabled) {
        std::vector<std::shared_ptr<ISamplableTexture>> inputColorTextures;
        for (int i = 0; i < m_forwardFrameBuffers.size(); i++) {
            inputColorTextures.emplace_back(m_forwardFrameBuffers[i]->getAttachment(0));
        }

        glowPass->updateDimensions(m_width, m_height,
                                   inputColorTextures,
                                   m_fxaaEnabled ? m_fxaaInputRenderPass :
                                       (!this->m_createOutputFBO ? m_device->getSwapChainRenderPass() : this->m_outputRenderPass),
                                   true);
        m_lastGlowTargetIsFxaa = m_fxaaEnabled;
    }
    if (m_fxaaEnabled && !m_fxaaMatsBuilt) {
        createFxaaMats();
        m_fxaaMatsBuilt = true;
    }

    updateLightBuffers(pointLights, spotLights, insideSpotLights);
    glowPass->assignFFXGlowUBOConsts(glow);
}

void RenderViewDeferredVLK::updateExternalDSes() {
    auto frameNum = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

    auto liquidDepthSamplable = m_liquidDepthFrameBuffers[frameNum]
        ? m_device->createSampledTexture(m_liquidDepthFrameBuffers[frameNum]->getDepthTexture(), false, false)
        : nullptr;


    auto &sceneUpdate = m_sceneWideDS->beginUpdate()
        .texture(1, m_lightFrameBuffers[frameNum]->getAttachment(0))
        .texture_depth(3, liquidDepthSamplable);

    m_gBufferDataDS->beginUpdate()
        .texture(0, m_gBufferFrameBuffers[frameNum]->getAttachment(1))
        .texture(1, m_gBufferFrameBuffers[frameNum]->getAttachment(0))
        .ubo(2, BufferChunkHelperVLK::cast(m_lightScreenSize));
}

static inline std::array<float,3> vec4ToArr3(const mathfu::vec4 &vec) {
    return {vec[0], vec[1], vec[2]};
}

RenderPassHelper RenderViewDeferredVLK::beginGBufferPass(CmdBufRecorder &frameBufCmd,
                                                  bool willExecuteSecondaryBuffs,
                                                  mathfu::vec4 &clearColor) {
    return frameBufCmd.beginRenderPass(willExecuteSecondaryBuffs,
                                       m_gBufferRenderPass,
                                       m_gBufferFrameBuffers[FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT],
                                       {0,0},
                                       {m_width, m_height},
                                       vec4ToArr3(clearColor));
}
void RenderViewDeferredVLK::doHiZOcclusionCulling(CmdBufRecorder &cmdBuf,
                                                    const mathfu::mat4 &viewProj,
                                                    const HGBufferVLK &aabbBuffer,
                                                    const HGBufferVLK &visibilityBuffer,
                                                    uint32_t objectCount) {

    bool invertZ = m_gBufferRenderPass->getInvertZ();
    auto frameNum = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;
    m_hiZOcclusionCulling->execute(cmdBuf, m_gBufferFrameBuffers[frameNum],
                                    viewProj, aabbBuffer, visibilityBuffer, objectCount, invertZ);
}

RenderPassHelper RenderViewDeferredVLK::beginLiquidDepthPass(CmdBufRecorder &cmd) {
    auto frameNum = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;
    return cmd.beginRenderPass(false,
                               m_liquidDepthRenderPass,
                               m_liquidDepthFrameBuffers[frameNum],
                               {0, 0},
                               {m_width, m_height},
                               {0.0f, 0.0f, 0.0f}); // clear color unused for depth-only
}

void RenderViewDeferredVLK::doLiquidHiZ(CmdBufRecorder &cmdBuf) {
    auto frameNum = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;
    m_liquidHiZ->execute(cmdBuf, m_liquidDepthFrameBuffers[frameNum]);
}

HGSamplableTexture RenderViewDeferredVLK::getLiquidDepthTexture() {
    auto frameNum = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;
    auto depthTex = m_liquidDepthFrameBuffers[frameNum]->getDepthTexture();
    return m_device->createSampledTexture(depthTex, false, false);
}

void RenderViewDeferredVLK::copyForwardColorToLiquidBackBuffer(CmdBufRecorder &cmdBuf) {
    auto frameNum = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

    auto srcSamplable = m_forwardFrameBuffers[frameNum]->getAttachment(0);
    auto srcTexVlk = std::dynamic_pointer_cast<GTextureVLK>(srcSamplable->getTexture());
    auto &dstTexVlk = m_liquidBackBufferTextures[frameNum];

    if (!srcTexVlk || !dstTexVlk) return;

    VkImage srcImage = srcTexVlk->texture.image;
    VkImage dstImage = dstTexVlk->texture.image;

    // Transition src: SHADER_READ_ONLY (forward pass finalLayout) → TRANSFER_SRC
    // Transition dst: UNDEFINED → TRANSFER_DST
    std::vector<VkImageMemoryBarrier> preBarriers(2);

    preBarriers[0] = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    preBarriers[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    preBarriers[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    preBarriers[0].oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    preBarriers[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    preBarriers[0].image = srcImage;
    preBarriers[0].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    preBarriers[1] = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    preBarriers[1].srcAccessMask = 0;
    preBarriers[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    preBarriers[1].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    preBarriers[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    preBarriers[1].image = dstImage;
    preBarriers[1].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    cmdBuf.recordPipelineImageBarrier(
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        preBarriers
    );

    // Blit src → dst
    VkImageBlit blitRegion = {};
    blitRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    blitRegion.srcOffsets[0] = {0, 0, 0};
    blitRegion.srcOffsets[1] = {(int32_t)m_width, (int32_t)m_height, 1};
    blitRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    blitRegion.dstOffsets[0] = {0, 0, 0};
    blitRegion.dstOffsets[1] = {(int32_t)m_width, (int32_t)m_height, 1};

    cmdBuf.blitImage(srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                     dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     blitRegion, VK_FILTER_NEAREST);

    // Transition src: TRANSFER_SRC → SHADER_READ_ONLY (initialLayout the next forward pass expects)
    // Transition dst: TRANSFER_DST → SHADER_READ (ready for liquid shader sampling)
    std::vector<VkImageMemoryBarrier> postBarriers(2);

    postBarriers[0] = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    postBarriers[0].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    postBarriers[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    postBarriers[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    postBarriers[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    postBarriers[0].image = srcImage;
    postBarriers[0].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    postBarriers[1] = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    postBarriers[1].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    postBarriers[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    postBarriers[1].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    postBarriers[1].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    postBarriers[1].image = dstImage;
    postBarriers[1].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    cmdBuf.recordPipelineImageBarrier(
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        postBarriers
    );

    // Update gBufferDataDS with the backbuffer copy for liquid shader sampling
    m_gBufferDataDS->beginUpdate()
        .texture(3, m_liquidBackBufferSamplable[frameNum]);
}

void RenderViewDeferredVLK::copyObjectIdPixelToBuffer(CmdBufRecorder &cmdBuf, int x, int y, VkBuffer dstBuffer) {
    if (!m_device->supportsSelection()) return;

    auto frameNum = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

    auto srcSamplable = m_forwardFrameBuffers[frameNum]->getAttachment(1);
    if (!srcSamplable) return;
    auto srcTexVlk = std::dynamic_pointer_cast<GTextureVLK>(srcSamplable->getTexture());
    if (!srcTexVlk) return;

    int clampedX = std::min(std::max(x, 0), (int)m_width - 1);
    int clampedY = std::min(std::max(y, 0), (int)m_height - 1);

    VkImage srcImage = srcTexVlk->texture.image;

    // The forward render pass declares finalLayout = SHADER_READ_ONLY_OPTIMAL for all color attachments,
    // so by the time this is called (after the last forward sub-pass of the frame has ended) that's the
    // layout the object-id attachment is guaranteed to be in.
    VkImageMemoryBarrier toTransferSrc = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    toTransferSrc.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    toTransferSrc.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    toTransferSrc.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    toTransferSrc.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    toTransferSrc.image = srcImage;
    toTransferSrc.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    cmdBuf.recordPipelineImageBarrier(
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        std::vector<VkImageMemoryBarrier>{toTransferSrc}
    );

    VkBufferImageCopy copyRegion = {};
    copyRegion.bufferOffset = 0;
    copyRegion.bufferRowLength = 0;
    copyRegion.bufferImageHeight = 0;
    copyRegion.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copyRegion.imageOffset = {clampedX, clampedY, 0};
    copyRegion.imageExtent = {1, 1, 1};

    vkCmdCopyImageToBuffer(cmdBuf.getNativeCmdBuffer(), srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           dstBuffer, 1, &copyRegion);

    // Transition back so the next (non-clearing) forward sub-pass this frame still sees the layout it expects.
    VkImageMemoryBarrier backToShaderRead = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    backToShaderRead.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    backToShaderRead.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    backToShaderRead.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    backToShaderRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    backToShaderRead.image = srcImage;
    backToShaderRead.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    cmdBuf.recordPipelineImageBarrier(
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        std::vector<VkImageMemoryBarrier>{backToShaderRead}
    );
}

void RenderViewDeferredVLK::doGBufferBarrier(CmdBufRecorder &frameBufCmd) {
    auto const &fb = m_gBufferFrameBuffers[FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT];

    // Barrier for first attachment (using forAttachmentRead = true to stay in attachment stage)
    addAttachmentToShaderReadBarrier(frameBufCmd, fb->getAttachment(0), true);

    // Barrier for second attachment
    addAttachmentToShaderReadBarrier(frameBufCmd, fb->getAttachment(1), true);
}
void RenderViewDeferredVLK::doLightPass(CmdBufRecorder &frameBufCmd) {
    ZoneScopedN("Light Pass");
    auto debugHelper = frameBufCmd.beginDebugLabel("Light Pass", {0, 0, 0.5f});


    auto frameNum = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

    auto renderPass = frameBufCmd.beginRenderPass(false, m_lightBufferPass,
                                m_lightFrameBuffers[frameNum],
                                {0,0},
                                {m_width, m_height},
                                {0,0,0});

    frameBufCmd.setViewPort(CmdBufRecorder::ViewportType::vp_usual);

    auto const pointLightCount = m_pointLightCounts[frameNum];
    auto const spotLightCount = m_spotLightCounts[frameNum];
    auto const insideSpotLightCount = m_insideSpotLightCounts[frameNum];
    if (pointLightCount > 0) {
        VkZone(frameBufCmd, "Point Lights")

        frameBufCmd.bindVertexBindings(m_quadVAO);
        frameBufCmd.bindMaterial(m_pointLightMats[frameNum]);
        frameBufCmd.drawIndexed(6, pointLightCount, 0, 0, 0);

         // for (int i = 0; i < pointLightCount; i++)
             // frameBufCmd.drawIndexed(6, 1, 0, i, 0);
    }

    {
        VkZone(frameBufCmd, "Spot Lights")

        if (spotLightCount > 0) {
            frameBufCmd.bindVertexBindings(m_spotVAO);
            frameBufCmd.bindMaterial(m_spotLightMats[frameNum]);
            frameBufCmd.drawIndexed((spotLightSegments) * 3 * 2, spotLightCount, 0, 0, 0);

             // for (int i = 0; i < spotLightCount; i++)
                 // frameBufCmd.drawIndexed((spotLightSegments) * 3 * 2, 1, 0, i, 0);
        }
        if (insideSpotLightCount > 0) {
            frameBufCmd.bindVertexBindings(m_quadVAO);
            frameBufCmd.bindMaterial(m_insideSpotLightMats[frameNum]);
            frameBufCmd.drawIndexed(6, insideSpotLightCount, 0, 0, 0);

            // for (int i = 0; i < insideSpotLightCount; i++)
                // frameBufCmd.drawIndexed(6, 1, 0, i, 0);
        }
    }
}

void RenderViewDeferredVLK::doDebugLightPass(CmdBufRecorder &frameBufCmd) {
    auto frameNum = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

    auto const pointLightCount = m_pointLightCounts[frameNum];
    auto const spotLightCount = m_spotLightCounts[frameNum];
    // if (pointLightCount > 0) {
    //     frameBufCmd.bindVertexBindings(m_quadVAO);
    //     frameBufCmd.bindMaterial(m_pointLightMats[frameNum]);
    //     for (int i = 0; i < pointLightCount; i++)
    //         frameBufCmd.drawIndexed(6, 1, 0, i, 0);
    // }
    if (spotLightCount > 0) {
        frameBufCmd.bindVertexBindings(m_spotVAO);
        frameBufCmd.bindMaterial(m_spotLightDebugMats[frameNum]);
        frameBufCmd.drawIndexed((spotLightSegments*3) + (spotLightSegments - 2) * 3, spotLightCount, 0, 0, 0);

        // for (int i = 0; i < spotLightCount; i++)
            // frameBufCmd.drawIndexed((spotLightSegments*3) + (spotLightSegments - 2) * 3, 1, 0, i, 0);
    }
}

RenderPassHelper RenderViewDeferredVLK::beginForwardPass(CmdBufRecorder &frameBufCmd, bool willExecuteSecondaryBuffs,
                                                         bool clearDepth, mathfu::vec4 &clearColor, bool clearColorBuf) {
    std::shared_ptr<GRenderPassVLK> renderPass;
    if (clearDepth) {
        renderPass = m_forwardRenderPass;           // clear color + clear depth
    } else if (clearColorBuf) {
        renderPass = m_forwardRenderPassNoDepthClear; // clear color + load depth
    } else {
        renderPass = m_forwardRenderPassNoClear;    // load color + load depth
    }

    return frameBufCmd.beginRenderPass(willExecuteSecondaryBuffs,
                                       renderPass,
                                       m_forwardFrameBuffers[FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT],
                                       {0,0},
                                       {m_width, m_height},
                                       vec4ToArr3(clearColor));
}

void RenderViewDeferredVLK::addAttachmentToShaderReadBarrier(CmdBufRecorder &cmdBuf, const HGSamplableTexture &texture, bool forAttachmentRead) {
    auto vkTexture = std::dynamic_pointer_cast<GTextureVLK>(texture->getTexture());
    bool isDepth = vkTexture->isDepthTexture();

    assert(vkTexture->getIsSamplable());

    VkImageSubresourceRange subresourceRange = {};
    // FBO depth formats always have a stencil aspect (D32_SFLOAT_S8_UINT / D24_UNORM_S8_UINT);
    // without separateDepthStencilLayouts, barriers must cover both aspects (VUID-VkImageMemoryBarrier-image-03320)
    subresourceRange.aspectMask = isDepth ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) : VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    subresourceRange.layerCount = 1;

    VkImageMemoryBarrier imgBarrier{};
    imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imgBarrier.subresourceRange = subresourceRange;
    imgBarrier.image = vkTexture->texture.image;

    // Set layout based on texture type
    if (isDepth) {
        imgBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        imgBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    } else {
        imgBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    // Set access masks and pipeline stages based on texture type and usage
    if (isDepth) {
        imgBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        imgBarrier.dstAccessMask = forAttachmentRead ? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT : VK_ACCESS_SHADER_READ_BIT;
    } else {
        imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imgBarrier.dstAccessMask = forAttachmentRead ? VK_ACCESS_COLOR_ATTACHMENT_READ_BIT : VK_ACCESS_SHADER_READ_BIT;
    }

    VkPipelineStageFlags srcStage = isDepth ? VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT : VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkPipelineStageFlags dstStage = forAttachmentRead ? srcStage : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    cmdBuf.recordPipelineImageBarrier(srcStage, dstStage, {imgBarrier});
}

void RenderViewDeferredVLK::addColorAttachmentToShaderReadBarrier(CmdBufRecorder &cmdBuf, const HGSamplableTexture &texture) {
    addAttachmentToShaderReadBarrier(cmdBuf, texture, false);
}
void RenderViewDeferredVLK::doPostGlow(CmdBufRecorder &frameBufCmd) {
    auto frameNum = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

    // Add barrier for the forward framebuffer before glow pass reads it
    addColorAttachmentToShaderReadBarrier(frameBufCmd, m_forwardFrameBuffers[frameNum]->getAttachment(0));

    glowPass->doPass(frameBufCmd);

    // Add barrier for the final Gauss pass output before final glow material reads it
    auto finalGaussFrameBuffer = glowPass->getFinalGaussOutputFrameBuffer(frameNum);
    addColorAttachmentToShaderReadBarrier(frameBufCmd, finalGaussFrameBuffer->getAttachment(0));
}

void RenderViewDeferredVLK::doOutputPass(CmdBufRecorder &frameBufCmd) {
    auto frameNum = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

    auto frameBuff = m_outputFrameBuffers[FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT];
    if (m_fxaaEnabled) {
        // Composite into the FXAA intermediate, then FXAA into the output FBO
        doComposite(frameBufCmd);

        auto passHelper = frameBufCmd.beginRenderPass(
            false,
            m_outputRenderPass,
            frameBuff,
            {0,0},
            {static_cast<unsigned int>(frameBuff->getWidth()), static_cast<unsigned int>(frameBuff->getHeight())},
            {0, 0, 0}
        );
        drawFxaaQuad(frameBufCmd, frameNum);
    } else {
        glowPass->doFinalPass(frameBufCmd, frameBuff);
    }
}


void RenderViewDeferredVLK::doPostFinal(CmdBufRecorder &bufCmd) {
    auto frameNum = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;
    
    glowPass->doFinalDraw(bufCmd);
}

void RenderViewDeferredVLK::doComposite(CmdBufRecorder &frameBufCmd) {
    auto frameNum = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

    // Composite forward color + glow into the FXAA intermediate
    glowPass->doFinalPass(frameBufCmd, m_fxaaInputFrameBuffers[frameNum]);

    // Make the composite visible to the FXAA pass (which may run on the swapchain cmd buffer)
    addColorAttachmentToShaderReadBarrier(frameBufCmd, m_fxaaInputFrameBuffers[frameNum]->getAttachment(0));
}

void RenderViewDeferredVLK::doPostFinalFxaa(CmdBufRecorder &bufCmd) {
    auto frameNum = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

    drawFxaaQuad(bufCmd, frameNum);
}

void RenderViewDeferredVLK::drawFxaaQuad(CmdBufRecorder &cmdBuf, int frameNum) {
    auto vulkanBindings = std::dynamic_pointer_cast<GVertexBufferBindingsVLK>(m_quadVAO);
    cmdBuf.bindVertexBuffers(vulkanBindings->getVertexBuffers());
    cmdBuf.bindIndexBuffer(vulkanBindings->getIndexBuffer());

    cmdBuf.bindPipeline(m_fxaaMats[frameNum]->getPipeline());
    cmdBuf.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, m_fxaaMats[frameNum]->getDescriptorSets());

    cmdBuf.setViewPort(CmdBufRecorder::ViewportType::vp_usual);
    cmdBuf.setDefaultScissors();
    cmdBuf.drawIndexed(6, 1, 0, 0);
}

void RenderViewDeferredVLK::createFxaaMats() {
    static const PipelineTemplate s_fxaaPipelineT = []{
        PipelineTemplate pipelineTemplate;
        pipelineTemplate.element = DrawElementMode::TRIANGLES;
        pipelineTemplate.depthWrite = false;
        pipelineTemplate.depthCulling = false;
        pipelineTemplate.backFaceCulling = false;
        pipelineTemplate.blendMode = EGxBlendEnum::GxBlend_Opaque;
        return pipelineTemplate;
    }();

    // Default view composites to the swapchain; output views to their output FBO
    auto targetPass = !this->m_createOutputFBO ? m_device->getSwapChainRenderPass() : this->m_outputRenderPass;

    for (int i = 0; i < IDevice::MAX_FRAMES_IN_FLIGHT; i++) {
        m_fxaaMats[i] = MaterialBuilderVLK::fromShader(m_device, {"drawQuad", "fxaa"},
                                                       {"forwardRendering", "forwardRendering"})
            .createPipeline(m_quadVAO, targetPass, s_fxaaPipelineT)
            .createDescriptorSet(0, [this](std::shared_ptr<GDescriptorSet> &ds) {
                ds->beginUpdate()
                    .ubo(0, BufferChunkHelperVLK::cast(m_fxaaVsParams))
                    .ubo(1, BufferChunkHelperVLK::cast(m_fxaaScreenSize))
                    .delayUpdate();
            })
            .createDescriptorSet(1, [this, i](std::shared_ptr<GDescriptorSet> &ds) {
                ds->beginUpdate()
                    .texture(0, m_fxaaInputFrameBuffers[i]->getAttachment(0));
            })
            .toMaterial();
    }
}

void RenderViewDeferredVLK::iterateOverOutputTextures(
    std::function<void(const std::array<std::shared_ptr<ISamplableTexture>, IDevice::MAX_FRAMES_IN_FLIGHT> &,
                       const std::string &, ITextureFormat)> callback) {

    //1. Color output
    if (m_createOutputFBO) {
        std::array<std::shared_ptr<ISamplableTexture>, IDevice::MAX_FRAMES_IN_FLIGHT> colorTextures;
        for (int i = 0; i < IDevice::MAX_FRAMES_IN_FLIGHT; i++)
            colorTextures[i] = m_outputFrameBuffers[i]->getAttachment(0);

        callback(colorTextures, "Color Texture", ITextureFormat::itRGBA);
    }

    //2. Light Buffer
    {
        std::array<std::shared_ptr<ISamplableTexture>, IDevice::MAX_FRAMES_IN_FLIGHT> normalBufferTextures;
        for (int i = 0; i < IDevice::MAX_FRAMES_IN_FLIGHT; i++)
            normalBufferTextures[i] = m_gBufferFrameBuffers[i]->getAttachment(0);

        callback(normalBufferTextures, "Normal buffer", ITextureFormat::itRGBA);
    }

    //3. Light Buffer
    {
        std::array<std::shared_ptr<ISamplableTexture>, IDevice::MAX_FRAMES_IN_FLIGHT> lightBufferTextures;
        for (int i = 0; i < IDevice::MAX_FRAMES_IN_FLIGHT; i++)
            lightBufferTextures[i] = m_lightFrameBuffers[i]->getAttachment(0);

        callback(lightBufferTextures, "Light buffer", ITextureFormat::itRGBA);
    }

    //4. Depth buffer
    {
        std::array<std::shared_ptr<ISamplableTexture>, IDevice::MAX_FRAMES_IN_FLIGHT> depthTextures;
        for (int i = 0; i < IDevice::MAX_FRAMES_IN_FLIGHT; i++)
            depthTextures[i] = m_device->createSampledTexture(m_forwardFrameBuffers[i]->getDepthTexture(), false, false);

        callback(depthTextures, "Depth buffer", ITextureFormat::itDepth32);
    }

}

void
RenderViewDeferredVLK::readRGBAPixels(int frameNumber, int x, int y, int width, int height, void *outputdata) {
    if (m_createOutputFBO) {
        m_outputFrameBuffers[frameNumber % IDevice::MAX_FRAMES_IN_FLIGHT]->readRGBAPixels(x,y,width,height,outputdata);
    }
}

std::shared_ptr<IVideoRecordingContext> RenderViewDeferredVLK::createVideoRecordingContext(uint32_t framebufferWidth, uint32_t framebufferHeight, uint32_t outputWidth, uint32_t outputHeight, const std::string &outputFilename) {
    if (!m_createOutputFBO) {
        // Cannot record video without output FBO
        return nullptr;
    }
    
    // Create the video recording context with separate framebuffer and output dimensions
    return std::make_shared<VideoRecordingContextVLK>(framebufferWidth, framebufferHeight, outputWidth, outputHeight, outputFilename, shared_from_this());
}

void RenderViewDeferredVLK::feedFrameToVideoRecording(std::shared_ptr<IVideoRecordingContext> context, int frameNumber) {
    if (!m_createOutputFBO || !context) {
        return;
    }
    
    auto vkContext = std::dynamic_pointer_cast<VideoRecordingContextVLK>(context);
    if (!vkContext) {
        return;
    }
    
    // Allocate buffer for pixel data (use framebuffer dimensions, not output dimensions)
    uint32_t width = vkContext->getFramebufferWidth();
    uint32_t height = vkContext->getFramebufferHeight();
    std::vector<uint8_t> pixelData(width * height * 4);
    
    // Read pixels from the framebuffer
    readRGBAPixels(frameNumber, 0, 0, width, height, pixelData.data());
    
    // Feed the frame to the video recording context (will resize if needed)
    vkContext->encodeFrame(frameNumber, pixelData.data());
}

void RenderViewDeferredVLK::updateLightBuffers(const std::vector<LocalLight> &pointLights,
                                               const std::vector<SpotLight> &spotLights,
                                               const std::vector<SpotLight> &insideSpotLights
                                               ) {
    bool recreateLightMat = false;

    auto frameNum = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

    {
        auto &pointLightBuffer = m_pointLightBuffers[frameNum];
        if (pointLightBuffer->getSize() < pointLights.size() * sizeof(LocalLight)) {
            pointLightBuffer = m_pointLightDataBuffer->getSubBuffer((pointLights.size() + 100) * sizeof(LocalLight));
            recreateLightMat = true;
        }

        pointLightBuffer->uploadData(pointLights.data(), pointLights.size() * sizeof(LocalLight));

        m_pointLightCounts[frameNum] = pointLights.size();
    }
    {
        auto &spotLightBuffer = m_spotLightBuffers[frameNum];
        if (spotLightBuffer->getSize() < spotLights.size()*sizeof(SpotLight)) {
            spotLightBuffer = m_spotLightDataBuffer->getSubBuffer((spotLights.size() + 100) * sizeof(SpotLight));
            recreateLightMat = true;
        }

        spotLightBuffer->uploadData(spotLights.data(), spotLights.size()*sizeof(SpotLight));

        m_spotLightCounts[frameNum] = spotLights.size();
    }
    {
        auto &insideSpotLightBuffer = m_insideSpotLightBuffers[frameNum];
        if (insideSpotLightBuffer->getSize() < insideSpotLights.size()*sizeof(SpotLight)) {
            insideSpotLightBuffer = m_spotLightDataBuffer->getSubBuffer((insideSpotLights.size() + 100) * sizeof(SpotLight));
            recreateLightMat = true;
        }

        insideSpotLightBuffer->uploadData(insideSpotLights.data(), insideSpotLights.size()*sizeof(SpotLight));

        m_insideSpotLightCounts[frameNum] = insideSpotLights.size();
    }

    if (recreateLightMat) {
        createLightBufferMats();
    }
}
