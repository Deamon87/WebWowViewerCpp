//
// Created by Deamon on 10/5/2023.
//

#include "RenderViewForwardVLK.h"

/*
 * RenderViewForwardVLK
 */

RenderViewForwardVLK::RenderViewForwardVLK(const HGDeviceVLK &device,
                                                                     const HGBufferVLK &uboBuffer,
                                                                     const HGVertexBufferBindings &quadVAO,
                                                                     bool createOutputFBO) : m_device(device), m_createOutputFBO(createOutputFBO) {
    glowPass = std::make_unique<FFXGlowPassVLK>(m_device, uboBuffer, quadVAO);
}

void RenderViewForwardVLK::createFrameBuffers() {
    {
        auto const dataFormat = {ITextureFormat::itRGBA};
        auto depthFormat = ITextureFormat::itDepth32;
        bool invertZ = true;

        m_mainRenderPass = m_device->getRenderPass(dataFormat,
                                                     depthFormat,
                                                     sampleCountToVkSampleCountFlagBits(m_device->getMaxSamplesCnt()),
                                                     invertZ, false,
                                                     true, true);


        for (auto &colorFrameBuffer: m_colorFrameBuffers) {
            colorFrameBuffer = std::make_shared<GFrameBufferVLK>(
                *m_device,
                dataFormat,
                depthFormat,
                nullptr,
                m_device->getMaxSamplesCnt(),
                invertZ,
                m_width, m_height
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

void RenderViewForwardVLK::update(int width, int height, float glow) {
    width = std::max<int>(1, width);
    height = std::max<int>(1, height);

    if (width != m_width || height != m_height) {
        m_width = std::max<int>(1, width);
        m_height = std::max<int>(1, height);

        this->createFrameBuffers();

        {
            std::vector<std::shared_ptr<ISamplableTexture>> inputColorTextures;
            for (int i = 0; i < m_colorFrameBuffers.size(); i++) {
                inputColorTextures.emplace_back(m_colorFrameBuffers[i]->getAttachment(0));
            }

            glowPass->updateDimensions(m_width, m_height,
                                       inputColorTextures,
                                       !this->m_createOutputFBO ? m_device->getSwapChainRenderPass() : this->m_outputRenderPass);
        }

        this->executeOnChange();
    }
    glowPass->assignFFXGlowUBOConsts(glow);
}

static inline std::array<float,3> vec4ToArr3(const mathfu::vec4 &vec) {
    return {vec[0], vec[1], vec[2]};
}

RenderPassHelper RenderViewForwardVLK::beginPass(CmdBufRecorder &frameBufCmd,
                                                                           const std::shared_ptr<GRenderPassVLK> &renderPass,
                                                                           bool willExecuteSecondaryBuffs,
                                                                           mathfu::vec4 &clearColor) {
    return frameBufCmd.beginRenderPass(willExecuteSecondaryBuffs,
                                       renderPass,
                                       m_colorFrameBuffers[m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT],
                                       {0,0},
                                       {m_width, m_height},
                                       vec4ToArr3(clearColor));
}

void RenderViewForwardVLK::doOutputPass(CmdBufRecorder &frameBufCmd) {
    auto frameBuff = m_outputFrameBuffers[m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT];

    glowPass->doFinalPass(frameBufCmd, frameBuff);
}

void RenderViewForwardVLK::doPostGlow(CmdBufRecorder &frameBufCmd) {
    glowPass->doPass(frameBufCmd);
}

void RenderViewForwardVLK::doPostFinal(CmdBufRecorder &bufCmd) {
    glowPass->doFinalDraw(bufCmd);
}

void RenderViewForwardVLK::iterateOverOutputTextures(
    std::function<void(const std::array<std::shared_ptr<ISamplableTexture>, IDevice::MAX_FRAMES_IN_FLIGHT> &,
                       const std::string &, ITextureFormat)> callback) {

    //1. Color output
    if (m_createOutputFBO) {
        std::array<std::shared_ptr<ISamplableTexture>, IDevice::MAX_FRAMES_IN_FLIGHT> colorTextures;
        for (int i = 0; i < IDevice::MAX_FRAMES_IN_FLIGHT; i++)
            colorTextures[i] = m_outputFrameBuffers[i]->getAttachment(0);

        callback(colorTextures, "Color Texture", ITextureFormat::itRGBA);
    }

    //2. Depth buffer
    {
        std::array<std::shared_ptr<ISamplableTexture>, IDevice::MAX_FRAMES_IN_FLIGHT> depthTextures;
        for (int i = 0; i < IDevice::MAX_FRAMES_IN_FLIGHT; i++)
            depthTextures[i] = m_device->createSampledTexture(m_colorFrameBuffers[i]->getDepthTexture(), false, false);

        callback(depthTextures, "Depth buffer", ITextureFormat::itDepth32);
    }

}

void
RenderViewForwardVLK::readRGBAPixels(int frameNumber, int x, int y, int width, int height, void *outputdata) {
    if (m_createOutputFBO) {
        m_outputFrameBuffers[frameNumber % IDevice::MAX_FRAMES_IN_FLIGHT]->readRGBAPixels(x,y,width,height,outputdata);
    }
}