//
// Created by Deamon on 10/5/2023.
//

#include "RenderViewForwardVLK.h"
#include "VideoRecordingContextVLK.h"
#include "../../../../gapi/vulkan/textures/GTextureVLK.h"
#include "../../../../gapi/interface/FrameContext.h"

/*
 * RenderViewForwardVLK
 */

RenderViewForwardVLK::RenderViewForwardVLK(const HGDeviceVLK &device,
                                                                     const HGBufferVLK &uboBuffer,
                                                                     const HGVertexBufferBindings &quadVAO,
                                                                     bool createOutputFBO) : m_device(device), m_createOutputFBO(createOutputFBO) {
    glowPass = std::make_unique<FFXGlowPassVLK>(m_device, uboBuffer, quadVAO);
}

void RenderViewForwardVLK::createFrameBuffers(bool skipFrameBufCreation) {
    const auto deviceMaxSamples = 1; //m_device->getMaxSamplesCnt();

    {
        auto const dataFormat = {ITextureFormat::itRGBA};
        auto depthFormat = ITextureFormat::itDepth32;

        bool invertZ = false;

        m_mainRenderPass = m_device->getRenderPass(dataFormat,
                                                   depthFormat,
                                                   sampleCountToVkSampleCountFlagBits(deviceMaxSamples),
                                                   invertZ, false,
                                                   true, true);


        if (!skipFrameBufCreation) {
            for (auto &colorFrameBuffer: m_colorFrameBuffers) {
                colorFrameBuffer = std::make_shared<GFrameBufferVLK>(
                    *m_device,
                    m_mainRenderPass,
                    nullptr,
                    m_width, m_height
                );
            }
        }
    }
    if (m_createOutputFBO && !skipFrameBufCreation) {
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

}

void RenderViewForwardVLK::update(int width, int height, float glow) {
    width = std::max<int>(1, width);
    height = std::max<int>(1, height);

    if (width != m_width || height != m_height) {
        m_width = std::max<int>(1, width);
        m_height = std::max<int>(1, height);

        this->createFrameBuffers(false);

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
                                       m_colorFrameBuffers[FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT],
                                       {0,0},
                                       {m_width, m_height},
                                       vec4ToArr3(clearColor));
}

void RenderViewForwardVLK::doOutputPass(CmdBufRecorder &frameBufCmd) {
    auto frameBuff = m_outputFrameBuffers[FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT];

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

    for (auto &frameBuff : m_outputFrameBuffers)
        if (frameBuff == nullptr) return;

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

std::shared_ptr<IVideoRecordingContext> RenderViewForwardVLK::createVideoRecordingContext(uint32_t framebufferWidth, uint32_t framebufferHeight, uint32_t outputWidth, uint32_t outputHeight, const std::string &outputFilename) {
    if (!m_createOutputFBO) {
        // Cannot record video without output FBO
        return nullptr;
    }
    
    // Create the video recording context with separate framebuffer and output dimensions
    return std::make_shared<VideoRecordingContextVLK>(framebufferWidth, framebufferHeight, outputWidth, outputHeight, outputFilename, shared_from_this());
}

void RenderViewForwardVLK::feedFrameToVideoRecording(std::shared_ptr<IVideoRecordingContext> context, int frameNumber) {
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