//
// Created by Deamon on 10/5/2023.
//

#include "RenderViewDeferredVLK.h"
#include "../../../../gapi/vulkan/materials/MaterialBuilderVLK.h"
#include "../../../../gapi/vulkan/buffers/CBufferChunkVLK.h"

/*
 * RenderViewDeferredVLK
 */

RenderViewDeferredVLK::RenderViewDeferredVLK(const HGDeviceVLK &device,
                                             const HGBufferVLK &uboBuffer,
                                             const HGBufferVLK &pointLightBuffer,
                                             const HGBufferVLK &spotLightBuffer,
                                             const std::shared_ptr<GDescriptorSet> &sceneWideDS,
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
                                             m_createOutputFBO(createOutputFBO) {
    glowPass = std::make_unique<FFXGlowPassVLK>(m_device, uboBuffer, quadVAO);

    for (auto &pointLightBuff : m_pointLightBuffers) {
        pointLightBuff = m_pointLightDataBuffer->getSubBuffer(100*sizeof(LocalLight));
    }
    for (auto &spotLightBuff : m_spotLightBuffers) {
        spotLightBuff = m_spotLightDataBuffer->getSubBuffer(100*sizeof(SpotLight));
    }

    createFrameBuffers();

    m_lightScreenSize = std::make_shared<CBufferChunkVLK<mathfu::vec4_packed>>(uboBuffer);
}

void RenderViewDeferredVLK::createFrameBuffers() {
    {
        auto const gBufferFormat = {
            ITextureFormat::itRGBA,        //Normals
            ITextureFormat::itFloat32,        //Depth packed Readable
        };

        auto const forwardBufferFormat = {
            ITextureFormat::itRGBA   //Color
        };

        auto const lightBufferFormat = {
            ITextureFormat::itRGBA16   //Color
        };


        auto depthFormat = ITextureFormat::itDepth32;
        bool invertZ = true;

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

        m_lightBufferPass = m_device->getRenderPass(lightBufferFormat,
                                                    depthFormat,
                                                    sampleCountToVkSampleCountFlagBits(gBufferPassSamples),
                                                    invertZ, false,
                                                    true, false);

        for (auto &gBufferFrameBuffer: m_gBufferFrameBuffers) {
            gBufferFrameBuffer = std::make_shared<GFrameBufferVLK>(
                *m_device,
                gBufferFormat,
                depthFormat,
                nullptr,
                gBufferPassSamples,
                invertZ,
                m_width, m_height
            );
        }

//        std::vector<uint8_t> attachmentsToCopy = {};
        for (int i = 0; i < m_forwardFrameBuffers.size(); i++) {
            m_forwardFrameBuffers[i] = std::make_shared<GFrameBufferVLK>(
                *m_device,
                forwardBufferFormat,
                depthFormat,
                m_gBufferFrameBuffers[i]->getDepthTexture(),
                forwardPassSamples, //m_device->getMaxSamplesCnt(),
                invertZ,
                m_width,
                m_height
            );
        }
        for (int i = 0; i < m_lightFrameBuffers.size(); i++) {
            m_lightFrameBuffers[i] = std::make_shared<GFrameBufferVLK>(
                *m_device,
                lightBufferFormat,
                depthFormat,
                m_gBufferFrameBuffers[i]->getDepthTexture(),
                gBufferPassSamples, //m_device->getMaxSamplesCnt(),
                invertZ,
                m_width,
                m_height
            );
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
                dataFormat,
                ITextureFormat::itNone,
                nullptr,
                1,
                invertZ,
                m_width, m_height
            );
        }
    }
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
        }, {{0, m_sceneWideDS}})
        .createPipeline(m_quadVAO, m_lightBufferPass, s_lightBufferPipelineT)
        .bindDescriptorSet(0, m_sceneWideDS)
        .createDescriptorSet(1, [&](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(0, m_pointLightBuffers[i])
                .texture(1, m_gBufferFrameBuffers[i]->getAttachment(1))
                .texture(2, m_gBufferFrameBuffers[i]->getAttachment(0))
                .ubo(3, BufferChunkHelperVLK::cast(m_lightScreenSize))
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
        }, {{0, m_sceneWideDS}})
        .createPipeline(m_quadVAO, m_lightBufferPass, s_lightBufferPipelineSpot)
        .bindDescriptorSet(0, m_sceneWideDS)
        .createDescriptorSet(1, [&](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(0, m_spotLightBuffers[i])
                .texture(1, m_gBufferFrameBuffers[i]->getAttachment(1))
                .texture(2, m_gBufferFrameBuffers[i]->getAttachment(0))
                .ubo(3, BufferChunkHelperVLK::cast(m_lightScreenSize))
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
            }, {{0, m_sceneWideDS}})
            .createPipeline(m_spotLineVAO, m_forwardRenderPass, s_spotLightDebug)
            .bindDescriptorSet(0, m_sceneWideDS)
            .createDescriptorSet(1, [&](std::shared_ptr<GDescriptorSet> &ds) {
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
                                   const std::vector<SpotLight> &spotLights
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

        this->createFrameBuffers();
        this->createLightBufferMats();

        {
            std::vector<std::shared_ptr<ISamplableTexture>> inputColorTextures;
            for (int i = 0; i < m_forwardFrameBuffers.size(); i++) {
                inputColorTextures.emplace_back(m_forwardFrameBuffers[i]->getAttachment(0));
            }

            glowPass->updateDimensions(m_width, m_height,
                                       inputColorTextures,
                                       !this->m_createOutputFBO ? m_device->getSwapChainRenderPass() : this->m_outputRenderPass);
        }

        this->executeOnChange();
    }
    if (!m_lightMatsCreated) {
        this->createLightBufferMats();
    }

    updateLightBuffers(pointLights, spotLights);
    glowPass->assignFFXGlowUBOConsts(glow);
}

void RenderViewDeferredVLK::setLightBuffers(const std::shared_ptr<GDescriptorSet> &sceneWideDS) {
    auto frameNum = m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

    sceneWideDS->beginUpdate()
        .texture(1, m_lightFrameBuffers[frameNum]->getAttachment(0));
}

static inline std::array<float,3> vec4ToArr3(const mathfu::vec4 &vec) {
    return {vec[0], vec[1], vec[2]};
}

RenderPassHelper RenderViewDeferredVLK::beginGBufferPass(CmdBufRecorder &frameBufCmd,
                                                  bool willExecuteSecondaryBuffs,
                                                  mathfu::vec4 &clearColor) {
    return frameBufCmd.beginRenderPass(willExecuteSecondaryBuffs,
                                       m_gBufferRenderPass,
                                       m_gBufferFrameBuffers[m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT],
                                       {0,0},
                                       {m_width, m_height},
                                       vec4ToArr3(clearColor));
}
void RenderViewDeferredVLK::doGBufferBarrier(CmdBufRecorder &frameBufCmd) {
    auto const &fb = m_gBufferFrameBuffers[m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT];

    {
        VkImageSubresourceRange subresourceRange = {};
        // Image only contains color data
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // Start at first mip level
        subresourceRange.baseMipLevel = 0;
        // We will transition on all mip levels
        subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        // The 2D texture only has one layer
        subresourceRange.layerCount = 1;

        VkImageMemoryBarrier imgBarrier{};
        imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imgBarrier.subresourceRange = subresourceRange;
        imgBarrier.image = std::dynamic_pointer_cast<GTextureVLK>(fb->getAttachment(0)->getTexture())->texture.image;
        imgBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imgBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

        frameBufCmd.recordPipelineImageBarrier(
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            {
                imgBarrier
            }
        );
    }
    {
        VkImageSubresourceRange subresourceRange = {};
        // Image only contains color data
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // Start at first mip level
        subresourceRange.baseMipLevel = 0;
        // We will transition on all mip levels
        subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        // The 2D texture only has one layer
        subresourceRange.layerCount = 1;

        VkImageMemoryBarrier imgBarrier{};
        imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imgBarrier.subresourceRange = subresourceRange;
        imgBarrier.image = std::dynamic_pointer_cast<GTextureVLK>(fb->getAttachment(1)->getTexture())->texture.image;
        imgBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imgBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

        frameBufCmd.recordPipelineImageBarrier(
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            {
                imgBarrier
            }
        );
    }
}
void RenderViewDeferredVLK::doLightPass(CmdBufRecorder &frameBufCmd) {

    auto frameNum = m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

    auto renderPass = frameBufCmd.beginRenderPass(false, m_lightBufferPass,
                                m_lightFrameBuffers[frameNum],
                                {0,0},
                                {m_width, m_height},
                                {0,0,0});

    auto const pointLightCount = m_pointLightCounts[frameNum];
    auto const spotLightCount = m_spotLightCounts[frameNum];
    if (pointLightCount > 0) {
        frameBufCmd.bindVertexBindings(m_quadVAO);
        frameBufCmd.bindMaterial(m_pointLightMats[frameNum]);
        for (int i = 0; i < pointLightCount; i++)
            frameBufCmd.drawIndexed(6, 1, 0, i, 0);
    }
    if (spotLightCount > 0) {
        frameBufCmd.bindVertexBindings(m_spotVAO);
        frameBufCmd.bindMaterial(m_spotLightMats[frameNum]);
//        frameBufCmd.drawIndexed((spotLightSegments*3) + (spotLightSegments - 2) * 3, spotLightCount, 0, 0, 0);

        for (int i = 0; i < spotLightCount; i++)
            frameBufCmd.drawIndexed((spotLightSegments*3) + (spotLightSegments - 2) * 3, 1, 0, i, 0);

    }
}

void RenderViewDeferredVLK::doDebugLightPass(CmdBufRecorder &frameBufCmd) {
    auto frameNum = m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

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
        //        frameBufCmd.drawIndexed((spotLightSegments*3) + (spotLightSegments - 2) * 3, spotLightCount, 0, 0, 0);

        for (int i = 0; i < spotLightCount; i++)
            frameBufCmd.drawIndexed((spotLightSegments*3) + (spotLightSegments - 2) * 3, 1, 0, i, 0);

    }
}

RenderPassHelper RenderViewDeferredVLK::beginForwardPass(CmdBufRecorder &frameBufCmd, bool willExecuteSecondaryBuffs,
                                                         bool clearDepth, mathfu::vec4 &clearColor) {
    return frameBufCmd.beginRenderPass(willExecuteSecondaryBuffs,
                                       clearDepth ? m_forwardRenderPass : m_forwardRenderPassNoDepthClear,
                                       m_forwardFrameBuffers[m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT],
                                       {0,0},
                                       {m_width, m_height},
                                       vec4ToArr3(clearColor));
}

void RenderViewDeferredVLK::doOutputPass(CmdBufRecorder &frameBufCmd) {
    auto frameBuff = m_outputFrameBuffers[m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT];

    glowPass->doFinalPass(frameBufCmd, frameBuff);
}

void RenderViewDeferredVLK::doPostGlow(CmdBufRecorder &frameBufCmd) {
    glowPass->doPass(frameBufCmd);
}

void RenderViewDeferredVLK::doPostFinal(CmdBufRecorder &bufCmd) {
    glowPass->doFinalDraw(bufCmd);
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

void RenderViewDeferredVLK::updateLightBuffers(const std::vector<LocalLight> &pointLights,
                                               const std::vector<SpotLight> &spotLights) {
    bool recreateLightMat = false;

    auto frameNum = m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

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

    if (recreateLightMat) {
        createLightBufferMats();
    }
}
