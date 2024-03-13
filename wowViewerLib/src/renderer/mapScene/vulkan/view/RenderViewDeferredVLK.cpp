//
// Created by Deamon on 10/5/2023.
//

#include "RenderViewDeferredVLK.h"

/*
 * RenderViewDeferredVLK
 */

RenderViewDeferredVLK::RenderViewDeferredVLK(const HGDeviceVLK &device,
                                                                     const HGBufferVLK &uboBuffer,
                                                                     const HGVertexBufferBindings &quadVAO,
                                                                     bool createOutputFBO) : m_device(device), m_createOutputFBO(createOutputFBO) {
    glowPass = std::make_unique<FFXGlowPassVLK>(m_device, uboBuffer, quadVAO);

    createFrameBuffers();
    {
        std::vector<std::shared_ptr<ISamplableTexture>> inputColorTextures;
        for (int i = 0; i < m_colorFrameBuffers.size(); i++) {
            inputColorTextures.emplace_back(m_colorFrameBuffers[i]->getAttachment(0));
        }

        glowPass->updateDimensions(m_width, m_height,
                                   inputColorTextures,
                                   !this->m_createOutputFBO ? m_device->getSwapChainRenderPass() : this->m_outputRenderPass);
    }
}

void RenderViewDeferredVLK::createFrameBuffers() {
    {
        auto const dataFormat = {
            ITextureFormat::itRGBA,         //Albedo
            ITextureFormat::itRGBA,         //Specular
            ITextureFormat::itRGBA          //Normal
        };

        auto depthFormat = ITextureFormat::itDepth32;
        bool invertZ = true;

        m_opaqueRenderPass = m_device->getRenderPass(dataFormat,
                                                     depthFormat,
                                                     sampleCountToVkSampleCountFlagBits(m_device->getMaxSamplesCnt()),
                                                     invertZ, false,
                                                     true, true);

        m_nonOpaqueRenderPass = m_device->getRenderPass({ITextureFormat::itRGBA},
                                                        depthFormat,
                                                        sampleCountToVkSampleCountFlagBits(m_device->getMaxSamplesCnt()),
                                                        invertZ, false,
                                                        false, false);

        for (auto &colorFrameBuffer: m_colorFrameBuffers) {
            colorFrameBuffer = std::make_shared<GFrameBufferVLK>(
                *m_device,
                dataFormat,
                depthFormat,
                m_device->getMaxSamplesCnt(),
                invertZ,
                m_width, m_height
            );
        }

        std::vector<uint8_t> attachmentsToCopy = {0};
        for (int i = 0; i < m_colorNonOpaqFrameBuffers.size(); i++) {
            m_colorNonOpaqFrameBuffers[i] = std::make_shared<GFrameBufferVLK>(
                m_colorFrameBuffers[i].get(),
                attachmentsToCopy,
                m_nonOpaqueRenderPass
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
                1,
                invertZ,
                m_width, m_height
            );
        }
    }

}

void RenderViewDeferredVLK::update(int width, int height, float glow) {
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

RenderPassHelper RenderViewDeferredVLK::beginOpaquePass(CmdBufRecorder &frameBufCmd,
                                                  bool willExecuteSecondaryBuffs,
                                                  mathfu::vec4 &clearColor) {
    return frameBufCmd.beginRenderPass(willExecuteSecondaryBuffs,
                                       m_opaqueRenderPass,
                                       m_colorFrameBuffers[m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT],
                                       {0,0},
                                       {m_width, m_height},
                                       vec4ToArr3(clearColor));
}
void RenderViewDeferredVLK::doOpaqueNonOpaqueBarrier(CmdBufRecorder &frameBufCmd) {
    auto const &fb = m_colorFrameBuffers[m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT];

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

RenderPassHelper RenderViewDeferredVLK::beginNonOpaquePass(CmdBufRecorder &frameBufCmd, bool willExecuteSecondaryBuffs, mathfu::vec4 &clearColor) {
    return frameBufCmd.beginRenderPass(willExecuteSecondaryBuffs,
                                       m_nonOpaqueRenderPass,
                                       m_colorNonOpaqFrameBuffers[m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT],
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

    //2. Depth buffer
    {
        std::array<std::shared_ptr<ISamplableTexture>, IDevice::MAX_FRAMES_IN_FLIGHT> depthTextures;
        for (int i = 0; i < IDevice::MAX_FRAMES_IN_FLIGHT; i++)
            depthTextures[i] = m_device->createSampledTexture(m_colorFrameBuffers[i]->getDepthTexture(), false, false);

        callback(depthTextures, "Depth buffer", ITextureFormat::itDepth32);
    }

}

void
RenderViewDeferredVLK::readRGBAPixels(int frameNumber, int x, int y, int width, int height, void *outputdata) {
    if (m_createOutputFBO) {
        m_outputFrameBuffers[frameNumber % IDevice::MAX_FRAMES_IN_FLIGHT]->readRGBAPixels(x,y,width,height,outputdata);
    }
}