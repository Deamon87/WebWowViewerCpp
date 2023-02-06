//
// Created by Deamon on 06.02.23.
//

#include "RenderPassHelper.h"
#include "CommandBufferRecorder.h"



// ----------------------------------------
//     RenderPassHelper
// ----------------------------------------

RenderPassHelper::RenderPassHelper(CmdBufRecorder &cmdBufRecorder,
                                                                   bool isAboutToExecSecondaryCMD,
                                                                   const std::shared_ptr<GRenderPassVLK> &renderPassVlk,
                                                                   const std::shared_ptr<GFrameBufferVLK> &frameBuffer,
                                                                   const std::array<int32_t, 2> &areaOffset,
                                                                   const std::array<uint32_t, 2> &areaSize,
                                                                   const std::array<float,3> &colorClearColor, float depthClear
) : m_cmdBufRecorder(cmdBufRecorder) {

    auto clearValues = renderPassVlk->produceClearColorVec(colorClearColor, depthClear);

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.pNext = NULL;
    renderPassInfo.renderPass = renderPassVlk->getRenderPass();
    renderPassInfo.framebuffer = frameBuffer->getFrameBuffer();
    renderPassInfo.renderArea.offset = {areaOffset[0], areaOffset[1]};
    renderPassInfo.renderArea.extent = {areaSize[0], areaSize[1]};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    auto &gCmdBuffer = m_cmdBufRecorder.m_gCmdBuffer;

    vkCmdBeginRenderPass(
        gCmdBuffer.m_cmdBuffer, &renderPassInfo,
        isAboutToExecSecondaryCMD ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : VK_SUBPASS_CONTENTS_INLINE
    );
}

RenderPassHelper::~RenderPassHelper() {
    auto &gCmdBuffer = m_cmdBufRecorder.m_gCmdBuffer;

    vkCmdEndRenderPass(gCmdBuffer.m_cmdBuffer);
    m_cmdBufRecorder.m_currentRenderPass = nullptr;
}


