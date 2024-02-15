//
// Created by Deamon on 3/17/2023.
//

#include "FFXGlowPassVLK.h"
#include "../../../../gapi/vulkan/buffers/CBufferChunkVLK.h"
#include "../../../../gapi/vulkan/materials/MaterialBuilderVLK.h"
#include "../../../../gapi/vulkan/GVertexBufferBindingsVLK.h"
#include "../../../frame/FrameProfile.h"
#include "../../../../gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferRecorder_inline.h"


FFXGlowPassVLK::FFXGlowPassVLK(const HGDeviceVLK &device, const HGBufferVLK &uboBuffer, const HGVertexBufferBindings &quadVAO) : m_device(device), m_drawQuadVao(quadVAO) {
    m_ffxGlowVs = std::make_shared<CBufferChunkVLK<mathfu::vec4_packed>>(uboBuffer);
    for (auto &ffxGaussPs : m_ffxGaussPSs) {
        ffxGaussPs = std::make_shared<CBufferChunkVLK<FXGauss::meshWideBlockPS>>(uboBuffer);
    }
    m_ffxGlowPS = std::make_shared<CBufferChunkVLK<mathfu::vec4_packed>>(uboBuffer);

    {
        auto const dataFormat = {ITextureFormat::itRGBA};
        m_renderPass = m_device->getRenderPass(dataFormat, ITextureFormat::itNone,
                                              VK_SAMPLE_COUNT_1_BIT,
//                                          sampleCountToVkSampleCountFlagBits(hDevice->getMaxSamplesCnt()),
                                              true, false, true, true);
    }

    {
        //Set constant values
        auto &ffxGlowVs = m_ffxGlowVs->getObject();
        ffxGlowVs = {1, 1, 0, 0};
        m_ffxGlowVs->save();

        static const std::array<std::array<float, 4>, 6> texOffsets = {{
                                                                           //X & Y
                                                                           {{-1, 0, 0, -1}},
                                                                           {{2, 2, -1, -1}},

                                                                           //X & Y
                                                                           {{-6, -1, 1, 6}},
                                                                           {{0, 0, 0, 0}},

                                                                           //X & Y
                                                                           {{0, 0, 0, 0}},
                                                                           {{10, 2, -2, -10}},
                                                                       }};

        for (int i = 0; i < GAUSS_PASS_COUNT; i++) {
            auto &ffxGlowPs1 = m_ffxGaussPSs[i]->getObject();
            std::copy(std::begin(texOffsets[i * 2]), std::end(texOffsets[i * 2]), std::begin(ffxGlowPs1.texOffsetX));
            std::copy(std::begin(texOffsets[i * 2 + 1]), std::end(texOffsets[i * 2 + 1]),
                      std::begin(ffxGlowPs1.texOffsetY));
            m_ffxGaussPSs[i]->save();
        }
    }
}

void FFXGlowPassVLK::updateDimensions(int width, int height,
                                      const std::vector<HGSamplableTexture> &inputColorTextures,
                                      const std::shared_ptr<GRenderPassVLK> &finalRenderPass) {

    if (m_width == width && m_height == height) return;

    m_width = width;
    m_height = height;

    createFrameBuffers(width, height);
    assert(inputColorTextures.size() == IDevice::MAX_FRAMES_IN_FLIGHT);

    PipelineTemplate glowPipelineTemplate;
    glowPipelineTemplate.element = DrawElementMode::TRIANGLES;
    glowPipelineTemplate.depthWrite = false;
    glowPipelineTemplate.depthCulling = false;
    glowPipelineTemplate.backFaceCulling = false;
    glowPipelineTemplate.blendMode = EGxBlendEnum::GxBlend_Opaque;

    for (int i = 0; i < IDevice::MAX_FRAMES_IN_FLIGHT; i++) {
        std::array<HGSamplableTexture, 4> inputTextures;
        //Fill input textures array
        inputTextures[0] = inputColorTextures[i];
        for (int j = 0; j < GAUSS_PASS_COUNT; j++)
            inputTextures[j + 1] = getTargetFrameBuffer(j, i)->getAttachment(0);


        // Create materials
        for (int j = 0; j < GAUSS_PASS_COUNT; j++) {
            ffxGaussMat[i][j] = createFFXGaussMat(
                m_ffxGlowVs,
                m_ffxGaussPSs[j],
                inputTextures[j],
                glowPipelineTemplate,
                m_renderPass
            );
        }

        ffxGlowMat[i] = createFFXGlowMat(m_ffxGlowVs,
                                         m_ffxGlowPS,
                                         inputColorTextures[i],
                                         getTargetFrameBuffer(GAUSS_PASS_COUNT-1, i)->getAttachment(0),
                                         glowPipelineTemplate,
                                         finalRenderPass
        );
    }
}


void FFXGlowPassVLK::drawMaterial (CmdBufRecorder& cmdBuf, const std::shared_ptr<IMaterial> &mat) {
    //1. Bind VBOs
    auto vulkanBindings = std::dynamic_pointer_cast<GVertexBufferBindingsVLK>(m_drawQuadVao);
    cmdBuf.bindVertexBuffers(vulkanBindings->getVertexBuffers());

    //2. Bind IBOs
    cmdBuf.bindIndexBuffer(vulkanBindings->getIndexBuffer());

    //3. Bind pipeline
    auto material = std::dynamic_pointer_cast<ISimpleMaterialVLK>(mat);
    cmdBuf.bindPipeline(material->getPipeline());

    //4. Bind Descriptor sets
    auto const &descSets = material->getDescriptorSets();
    cmdBuf.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, descSets);

    //7. Draw the mesh
    cmdBuf.drawIndexed(6, 1, 0, 0);
}

void FFXGlowPassVLK::doPass(CmdBufRecorder &frameBufCmd) {
    ZoneScoped;
    auto currentFrame = m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;
    {

        for (int i = 0; i < GAUSS_PASS_COUNT; i++) {
            auto passHelper = frameBufCmd.beginRenderPass(
                false,
                m_renderPass,
                getTargetFrameBuffer(i, currentFrame),
                {0,0},
                {m_width >> 2, m_height >> 2},
                {0, 0, 0}//todo
            );
            frameBufCmd.setViewPort(CmdBufRecorder::ViewportType::vp_usual);
            frameBufCmd.setDefaultScissors();
            drawMaterial(frameBufCmd, ffxGaussMat[currentFrame][i]);
        }
    }
}
void FFXGlowPassVLK::doFinalDraw(CmdBufRecorder &finalBufCmd) {
    auto currentFrame = m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

    {
        finalBufCmd.setViewPort(CmdBufRecorder::ViewportType::vp_usual);
        finalBufCmd.setDefaultScissors();
        drawMaterial(finalBufCmd, ffxGlowMat[currentFrame]);
    }
}
void FFXGlowPassVLK::doFinalPass(CmdBufRecorder &finalBufCmd, const std::shared_ptr<GFrameBufferVLK> &frameBuff) {
    auto passHelper = finalBufCmd.beginRenderPass(
        false,
        m_renderPass,
        frameBuff,
        {0,0},
        {static_cast<unsigned int>(frameBuff->getWidth()), static_cast<unsigned int>(frameBuff->getHeight())},
        {0, 0, 0}//todo
    );

    doFinalDraw(finalBufCmd);
}

void FFXGlowPassVLK::assignFFXGlowUBOConsts(float glow) {
    ZoneScoped;

    auto &ffxGlowPS = m_ffxGlowPS->getObject();
    ffxGlowPS = {1,1,0,glow};
    m_ffxGlowPS->save();
}

void FFXGlowPassVLK::createFrameBuffers(int m_width, int m_height) {
    {
        auto const dataFormat = {ITextureFormat::itRGBA};
        int targetWidth = m_width >> 2;
        int targetHeight = m_height >> 2;

        for (auto &colorFrameBuffer: m_GlowFrameB1) {
            colorFrameBuffer = std::make_shared<GFrameBufferVLK>(
                *m_device,
                dataFormat,
                ITextureFormat::itNone,
                1,
                false,
                targetWidth, targetHeight
            );
        }
        for (auto &colorFrameBuffer: m_GlowFrameB2) {
            colorFrameBuffer = std::make_shared<GFrameBufferVLK>(
                *m_device,
                dataFormat,
                ITextureFormat::itNone,
                1,
                false,
                targetWidth, targetHeight
            );
        }
    }
}

std::shared_ptr<GFrameBufferVLK> FFXGlowPassVLK::getTargetFrameBuffer(int GAUSS_PASS_I, int frameInFlightI) {
    return (GAUSS_PASS_I & 1) ?
        m_GlowFrameB1[frameInFlightI] :
        m_GlowFrameB2[frameInFlightI];
}

std::shared_ptr<IMaterial>
FFXGlowPassVLK::createFFXGaussMat(
    const std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> &ffxGaussVs,
    const std::shared_ptr<IBufferChunk<FXGauss::meshWideBlockPS>> &ffxGaussPS,
    const HGSamplableTexture &texture,
    const PipelineTemplate &pipelineTemplate,
    const std::shared_ptr<GRenderPassVLK> &targetRenderPass) {

    auto material = MaterialBuilderVLK::fromShader(m_device, {"drawQuad", "ffxgauss4"}, {"forwardRendering"})
        .createPipeline(m_drawQuadVao, targetRenderPass, pipelineTemplate)
        .createDescriptorSet(0, [&ffxGaussVs, &ffxGaussPS](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(ffxGaussVs))
                .ubo(1, BufferChunkHelperVLK::cast(ffxGaussPS)).delayUpdate();
        })
        .createDescriptorSet(1, [texture](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(0, texture);
        })
        .toMaterial();

    return material;
}

std::shared_ptr<IMaterial>
FFXGlowPassVLK::createFFXGlowMat(
    const std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> &ffxGlowVs,
    const std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> &ffxGlowPS,
    const HGSamplableTexture &screenTex,
    const HGSamplableTexture &blurTex,
    const PipelineTemplate &pipelineTemplate,
    const std::shared_ptr<GRenderPassVLK> &targetRenderPass) {

    auto material = MaterialBuilderVLK::fromShader(m_device, {"drawQuad", "ffxglow"}, {"forwardRendering"})
        .createPipeline(m_drawQuadVao, targetRenderPass, pipelineTemplate)
        .createDescriptorSet(0, [&ffxGlowVs, &ffxGlowPS](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(ffxGlowVs))
                .ubo(1, BufferChunkHelperVLK::cast(ffxGlowPS)).delayUpdate();
        })
        .createDescriptorSet(1, [screenTex, blurTex](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(0, screenTex)
                .texture(1, blurTex);
        })
        .toMaterial();

    return material;
}


