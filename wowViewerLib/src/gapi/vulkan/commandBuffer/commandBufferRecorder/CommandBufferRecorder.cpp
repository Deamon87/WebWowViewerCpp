//
// Created by Deamon on 06.02.23.
//

#include "CommandBufferRecorder.h"
#include "../../GPipelineVLK.h"

// ----------------------------------------
//     CmdBufRecorder
// ----------------------------------------

CmdBufRecorder::CmdBufRecorder(GCommandBuffer &cmdBuffer, const std::shared_ptr<GRenderPassVLK> &renderPass) : m_gCmdBuffer(cmdBuffer) {
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
CmdBufRecorder::~CmdBufRecorder() {
    if (m_gCmdBuffer.m_isPrimary && m_currentRenderPass != nullptr) {
        std::cerr << "currentRenderPass is not null. RenderPass was not closed" << std::endl;
    }

    if (vkEndCommandBuffer(m_gCmdBuffer.m_cmdBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}


uint32_t CmdBufRecorder::getQueueFamily() {
    return m_gCmdBuffer.m_queueFamilyIndex;
}


RenderPassHelper CmdBufRecorder::beginRenderPass(
    bool isAboutToExecSecondaryCMD,
    const std::shared_ptr<GRenderPassVLK> &renderPassVlk,
    const std::shared_ptr<GFrameBufferVLK> &frameBuffer,
    const std::array<int32_t, 2> &areaOffset,
    const std::array<uint32_t, 2> &areaSize,
    const std::array<float,3> &colorClearColor, float depthClear
) {
    if (m_currentRenderPass != nullptr) {
        throw std::runtime_error("tried to start render pass with another pass being active already");
    }

    m_currentRenderPass = renderPassVlk;
    return RenderPassHelper(
        *this,
        isAboutToExecSecondaryCMD,
        renderPassVlk, frameBuffer, areaOffset, areaSize, colorClearColor, depthClear
    );
}

void CmdBufRecorder::bindDescriptorSet(uint32_t bindIndex, const std::shared_ptr<GDescriptorSet> &descriptorSet) {
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

void CmdBufRecorder::bindPipeline(std::shared_ptr<GPipelineVLK> &pipeline) {
    if (m_currentPipeline == pipeline) return;

    vkCmdBindPipeline(m_gCmdBuffer.m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipeline());

    m_currentPipeline = pipeline;
}

void CmdBufRecorder::recordPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                           const std::vector<VkImageMemoryBarrier> &imageBarrierData) {
    vkCmdPipelineBarrier(
        m_gCmdBuffer.m_cmdBuffer,
        srcStageMask,
        dstStageMask,
        0,
        0, nullptr,
        0, nullptr,
        imageBarrierData.size(), imageBarrierData.data());
}

void CmdBufRecorder::copyBufferToImage(VkBuffer buffer, VkImage image, const std::vector<VkBufferImageCopy> &regions) {
    vkCmdCopyBufferToImage(
        m_gCmdBuffer.m_cmdBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        static_cast<uint32_t>(regions.size()),
        regions.data());
}

void CmdBufRecorder::   submitBufferUploads(const std::shared_ptr<GBufferVLK> &bufferVLK) {
    auto submitRecords = bufferVLK->getSubmitRecords();

    if (submitRecords.get().empty())
        return;

    vkCmdCopyBuffer(m_gCmdBuffer.m_cmdBuffer,
                    bufferVLK->getCPUBuffer(),
                    bufferVLK->getGPUBuffer(),
                    submitRecords.get().size(),
                    submitRecords.get().data());
}
