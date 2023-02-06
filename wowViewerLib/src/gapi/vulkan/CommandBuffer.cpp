//
// Created by Deamon on 05.02.23.
//

#include "CommandBuffer.h"
#include "GRenderPassVLK.h"
#include "GFrameBufferVLK.h"
#include "GPipelineVLK.h"

GCommandBuffer::GCommandBuffer(GDeviceVLK &deviceVlk, VkCommandPool commandPool, bool isPrimary) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(deviceVlk.getVkDevice(), &allocInfo, &m_cmdBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

GCommandBuffer::CmdBufRecorder GCommandBuffer::beginRecord(const std::shared_ptr<GRenderPassVLK> &renderPass) {
    //If renderPass != nullptr -> it means this is a secondary command buffer, that needs to continue renderPass, that's going on outside
    if (m_isPrimary && renderPass != nullptr) {
        std::cerr << "tried to continue renderpass in primary buffer " << std::endl;
        throw std::runtime_error("tried to continue renderpass in primary buffer ");
    }

    return CmdBufRecorder(*this, renderPass);
}

// ----------------------------------------
//     CmdBufRecorder
// ----------------------------------------

GCommandBuffer::CmdBufRecorder::CmdBufRecorder(GCommandBuffer &cmdBuffer, const std::shared_ptr<GRenderPassVLK> &renderPass) : m_gCmdBuffer(cmdBuffer) {
    VkCommandBufferInheritanceInfo bufferInheritanceInfo;

    //If not nullptr -> it means this is a secondary command buffer, that needs to continue renderPass, that's going on outside
    if (renderPass != nullptr) {
        bufferInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        bufferInheritanceInfo.pNext = nullptr;
        bufferInheritanceInfo.renderPass = renderPass->getRenderPass();
        bufferInheritanceInfo.subpass = 0;
        bufferInheritanceInfo.framebuffer = VK_NULL_HANDLE;
        bufferInheritanceInfo.occlusionQueryEnable = false;
        bufferInheritanceInfo.queryFlags = 0;
        bufferInheritanceInfo.pipelineStatistics = 0;

        m_currentRenderPass = renderPass;
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pNext = NULL;
    beginInfo.pInheritanceInfo = (renderPass != nullptr) ? &bufferInheritanceInfo : nullptr;

    if (vkBeginCommandBuffer(m_gCmdBuffer.m_cmdBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
}
GCommandBuffer::CmdBufRecorder::~CmdBufRecorder() {
    if (m_gCmdBuffer.m_isPrimary && m_currentRenderPass != nullptr) {
        std::cerr << "currentRenderPass is not null. RenderPass was not closed" << std::endl;
    }

    if (vkEndCommandBuffer(m_gCmdBuffer.m_cmdBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}


GCommandBuffer::CmdBufRecorder::RenderPassHelper GCommandBuffer::CmdBufRecorder::beginRenderPass(
    bool isAboutToExecSecondaryCMD,
    const std::shared_ptr<GRenderPassVLK> &renderPassVlk,
    const std::shared_ptr<GFrameBufferVLK> &frameBuffer,
    const std::array<int32_t, 2> areaOffset,
    const std::array<uint32_t, 2> areaSize,
    const std::array<float,3> &colorClearColor, float depthClear
) {
    if (m_currentRenderPass != nullptr) {
        throw std::runtime_error("tried to start render pass with another pass being active already");
    }

    m_currentRenderPass = renderPassVlk;
    return GCommandBuffer::CmdBufRecorder::RenderPassHelper(
        *this,
        isAboutToExecSecondaryCMD,
        renderPassVlk, frameBuffer, areaOffset, areaSize, colorClearColor, depthClear
    );
}

void GCommandBuffer::CmdBufRecorder::bindDescriptorSet(uint32_t bindIndex, std::shared_ptr<GDescriptorSet> &descriptorSet) {
    //TODO: bindpoints: VK_PIPELINE_BIND_POINT_GRAPHICS and others
    //Which leads to three separate states for:
    // VK_PIPELINE_BIND_POINT_GRAPHICS, VK_PIPELINE_BIND_POINT_COMPUTE, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR
    // Also, implement "Pipeline Layout Compatibility" thing from spec

    if (m_currentDescriptorSet[bindIndex] == descriptorSet) return;

    auto vkDescSet = descriptorSet->getDescSet();
    constexpr uint32_t vkDescCnt = 1;

    vkCmdBindDescriptorSets(m_gCmdBuffer.m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_currentPipeline->getLayout(), bindIndex, vkDescCnt, &vkDescSet, 0, nullptr);
}

void GCommandBuffer::CmdBufRecorder::bindPipeline(std::shared_ptr<GPipelineVLK> &pipeline) {
    if (m_currentPipeline == pipeline) return;

    vkCmdBindPipeline(m_gCmdBuffer.m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipeline());

    m_currentPipeline = pipeline;
}

// ----------------------------------------
//     RenderPassHelper
// ----------------------------------------

GCommandBuffer::CmdBufRecorder::RenderPassHelper::RenderPassHelper(CmdBufRecorder &cmdBufRecorder,
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

GCommandBuffer::CmdBufRecorder::RenderPassHelper::~RenderPassHelper() {
    auto &gCmdBuffer = m_cmdBufRecorder.m_gCmdBuffer;

    vkCmdEndRenderPass(gCmdBuffer.m_cmdBuffer);
    m_cmdBufRecorder.m_currentRenderPass = nullptr;
}

