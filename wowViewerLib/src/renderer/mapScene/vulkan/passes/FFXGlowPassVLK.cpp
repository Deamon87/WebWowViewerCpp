//
// Created by Deamon on 3/17/2023.
//

#include "FFXGlowPassVLK.h"
#include "../../../../gapi/vulkan/buffers/CBufferChunkVLK.h"
#include "../../../../gapi/vulkan/materials/MaterialBuilderVLK.h"
#include "../../../../gapi/vulkan/GVertexBufferBindingsVLK.h"
#include "../../../frame/FrameProfile.h"
#include "../../../../gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferRecorder_inline.h"
#include "../../../../gapi/interface/FrameContext.h"
#include "../../../../gapi/vulkan/textures/GTextureVLK.h"


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


        updateFsUBO(1, 1);
    }
}

void FFXGlowPassVLK::updateFsUBO(int width, int height) {
    //Set constant values
    auto &ffxGlowVs = m_ffxGlowVs->getObject();
    ffxGlowVs = {1.0f, 1.0f, 0.0f, 0.0f};
    m_ffxGlowVs->save();

    static const std::array<mathfu::vec4, 6> texOffsets = {{
        //X & Y
        {-1.5f, 0.5f, 2.667f, -1.5f},
        {1.498f,1.498f,-0.499f,-0.499f},

        //X & Y
        {-2.5f, -0.5f, 0.5f, 2.5f},
        {0, 0, 0, 0},

        //X & Y
        {0, 0, 0, 0},
        {2.505f, 0.501f, -0.501f, -2.505f},
    }};

    for (int i = 0; i < GAUSS_PASS_COUNT; i++) {
        auto &ffxGlowPs1 = m_ffxGaussPSs[i]->getObject();
        const auto &x_vec = texOffsets[i * 2];
        const auto &y_vec = texOffsets[(i * 2) + 1];

        int targetWidth = (i == 0) ? width : (width >> 2);
        int targetHeight = (i == 0) ? height : (height >> 2);

        ffxGlowPs1.texOffsetX = x_vec * (1.0f/(float)targetWidth);
        ffxGlowPs1.texOffsetY = y_vec * (1.0f/(float)targetHeight);

        m_ffxGaussPSs[i]->save();
    }
}

void FFXGlowPassVLK::updateDimensions(int width, int height,
                                      const std::vector<HGSamplableTexture> &inputColorTextures,
                                      const std::shared_ptr<GRenderPassVLK> &finalRenderPass,
                                      bool force) {

    if (!force && m_width == width && m_height == height) return;

    m_width = width;
    m_height = height;

    createFrameBuffers(width, height);
    updateFsUBO(width, height);

    assert(inputColorTextures.size() == IDevice::MAX_FRAMES_IN_FLIGHT);

    PipelineTemplate glowPipelineTemplate;
    glowPipelineTemplate.element = DrawElementMode::TRIANGLES;
    glowPipelineTemplate.depthWrite = false;
    glowPipelineTemplate.depthCulling = false;
    glowPipelineTemplate.backFaceCulling = false;
    glowPipelineTemplate.blendMode = EGxBlendEnum::GxBlend_Opaque;

    // FBO attachments default to a NEAREST sampler (right for gbuffer data, wrong for blur):
    // the gauss kernel's half-texel taps assume bilinear filtering, and the final composite
    // upsamples the quarter-res blur 4x — with NEAREST that shows as visible 4x4 squares.
    // Wrap the whole glow chain in linear samplers.
    auto asLinearSampled = [this](const HGSamplableTexture &t) -> HGSamplableTexture {
        return m_device->createSampledTexture(t->getTexture(), false, false);
    };

    for (int i = 0; i < IDevice::MAX_FRAMES_IN_FLIGHT; i++) {
        std::array<HGSamplableTexture, GAUSS_PASS_COUNT+1> inputTextures;
        //Fill input textures array
        inputTextures[0] = asLinearSampled(inputColorTextures[i]);
        for (int j = 0; j < GAUSS_PASS_COUNT; j++)
            inputTextures[j + 1] = asLinearSampled(getTargetFrameBuffer(j, i)->getAttachment(0));


        // Create materials
        for (int j = 0; j < GAUSS_PASS_COUNT; j++) {
            ffxGaussMat[i][j] = createFFXGaussMat(
                m_ffxGlowVs,
                m_ffxGaussPSs[j],
                j == 0,
                inputTextures[j],
                glowPipelineTemplate,
                m_renderPass
            );
        }

        ffxGlowMat[i] = createFFXGlowMat(m_ffxGlowVs,
                                         m_ffxGlowPS,
                                         inputTextures[0],
                                         inputTextures[GAUSS_PASS_COUNT],
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
    auto currentFrame = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;
    {

        for (int i = 0; i < GAUSS_PASS_COUNT; i++) {
            if (i > 0) {
                auto previousFrameBuffer = getTargetFrameBuffer(i - 1, currentFrame);
                auto previousTexture = std::dynamic_pointer_cast<GTextureVLK>(previousFrameBuffer->getAttachment(0)->getTexture());
                
                VkImageSubresourceRange subresourceRange = {};
                subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                subresourceRange.baseMipLevel = 0;
                subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
                subresourceRange.layerCount = 1;

                VkImageMemoryBarrier imgBarrier{};
                imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                imgBarrier.subresourceRange = subresourceRange;
                imgBarrier.image = previousTexture->texture.image;
                imgBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                frameBufCmd.recordPipelineImageBarrier(
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                    {imgBarrier}
                );
            }

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
    auto currentFrame = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

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
        int targetWidth = m_width >> 2;
        int targetHeight = m_height >> 2;

        for (auto &colorFrameBuffer: m_GlowFrameB1) {
            colorFrameBuffer = std::make_shared<GFrameBufferVLK>(
                *m_device,
                m_renderPass,
                nullptr,
                targetWidth, targetHeight
            );
        }
        for (auto &colorFrameBuffer: m_GlowFrameB2) {
            colorFrameBuffer = std::make_shared<GFrameBufferVLK>(
                *m_device,
                m_renderPass,
                nullptr,
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

std::shared_ptr<GFrameBufferVLK> FFXGlowPassVLK::getFinalGaussOutputFrameBuffer(int frameInFlightI) {
    return getTargetFrameBuffer(GAUSS_PASS_COUNT - 1, frameInFlightI);
}

std::shared_ptr<IMaterial>
FFXGlowPassVLK::createFFXGaussMat(
    const std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> &ffxGaussVs,
    const std::shared_ptr<IBufferChunk<FXGauss::meshWideBlockPS>> &ffxGaussPS,
    bool isInitCopyMat,
    const HGSamplableTexture &texture,
    const PipelineTemplate &pipelineTemplate,
    const std::shared_ptr<GRenderPassVLK> &targetRenderPass) {

    auto material = MaterialBuilderVLK::fromShader(m_device,
                                                   {"drawQuad", isInitCopyMat ? "ffxgauss4_copy" : "ffxgauss4"},
                                                   {"forwardRendering", "forwardRendering"})
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

    auto material = MaterialBuilderVLK::fromShader(m_device, {"drawQuad", "ffxglow"}, {"forwardRendering", "forwardRendering"})
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


