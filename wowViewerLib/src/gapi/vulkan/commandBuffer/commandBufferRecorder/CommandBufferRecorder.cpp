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

    createViewPortTypes(areaOffset, areaSize, renderPassVlk->getInvertZ());
    createDefaultScissors(areaOffset, areaSize);

    m_currentRenderPass = renderPassVlk;
    return RenderPassHelper(
        *this,
        isAboutToExecSecondaryCMD,
        renderPassVlk, frameBuffer, areaOffset, areaSize, colorClearColor, depthClear
    );
}

void CmdBufRecorder::bindDescriptorSet(VkPipelineBindPoint bindPoint, uint32_t bindIndex, const std::shared_ptr<GDescriptorSet> &descriptorSet) {
    //TODO: bindpoints: VK_PIPELINE_BIND_POINT_GRAPHICS and others
    //Which leads to three separate states for:
    // VK_PIPELINE_BIND_POINT_GRAPHICS, VK_PIPELINE_BIND_POINT_COMPUTE, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR
    // Also, implement "Pipeline Layout Compatibility" thing from spec

//    if (m_currentDescriptorSet[bindIndex] == descriptorSet) return;

    auto pDescriptorSet = descriptorSet.get();

    if (m_currentDescriptorSet[bindIndex] != pDescriptorSet) {
        auto vkDescSet = pDescriptorSet->getDescSet();
        constexpr uint32_t vkDescCnt = 1;


        std::array<uint32_t, 16> dynamicOffsets;
        uint32_t dynamicOffsetsSize;
        pDescriptorSet->getDynamicOffsets(dynamicOffsets, dynamicOffsetsSize);

        vkCmdBindDescriptorSets(m_gCmdBuffer.m_cmdBuffer, bindPoint,
                                m_currentPipeline->getLayout(),
                                bindIndex,
                                vkDescCnt, &vkDescSet,
                                dynamicOffsetsSize, dynamicOffsetsSize > 0 ? dynamicOffsets.data() : nullptr);

        m_currentDescriptorSet[bindIndex] = pDescriptorSet;
    }
}

CommandBufferDebugLabel CmdBufRecorder::beginDebugLabel(const std::string &labelName, const std::array<float, 4> &colors) {
    return CommandBufferDebugLabel(
        *this,
        labelName,
        colors
    );
}

void CmdBufRecorder::bindIndexBuffer(const std::shared_ptr<IBuffer> &buffer) {
    auto bufferVlk = std::dynamic_pointer_cast<IBufferVLK>(buffer);
    if (m_currentIndexBuffer!= nullptr && m_currentIndexBuffer->getGPUBuffer() == bufferVlk->getGPUBuffer() &&
        m_currentIndexBuffer->getOffset() == bufferVlk->getOffset()) return;

    VkDeviceSize offset = bufferVlk->getOffset();
    vkCmdBindIndexBuffer(m_gCmdBuffer.m_cmdBuffer, bufferVlk->getGPUBuffer(), offset, VK_INDEX_TYPE_UINT16);

    m_currentIndexBuffer = bufferVlk;
}
void CmdBufRecorder::bindVertexBuffers(const std::vector<std::shared_ptr<IBuffer>> &buffers){
    if (buffers.empty()) return;

    int firstBinding = 0;

    assert(buffers.size() <= MAX_VERTEX_BUFFERS_PER_DRAWCALL);

    std::array<VkBuffer, MAX_VERTEX_BUFFERS_PER_DRAWCALL> vbos = {};
    std::array<VkDeviceSize, MAX_VERTEX_BUFFERS_PER_DRAWCALL> offsets = {};
    int vboCnt = 0;
    int offsetCnt = 0;


    for (int i = 0; i < buffers.size(); i++) {
        auto const &bufferVlk = std::dynamic_pointer_cast<IBufferVLK>(buffers[i]);
        auto pbufferVlk = bufferVlk.get();

        //firstBinding == i <- means we can only skip the start of the list;
        if (firstBinding == i && m_currentVertexBuffers[i] != nullptr &&
                                 m_currentVertexBuffers[i]->getGPUBuffer() == pbufferVlk->getGPUBuffer() &&
                                 m_currentVertexBuffers[i]->getOffset() == pbufferVlk->getOffset()) {
            firstBinding++;
            continue;
        }

        vbos[vboCnt++] = pbufferVlk->getGPUBuffer();
        offsets[offsetCnt++] = pbufferVlk->getOffset();

        m_currentVertexBuffers[i] = bufferVlk;
    }

    int bindingCount = vboCnt;
    if (vboCnt == 0) return;

    vkCmdBindVertexBuffers(m_gCmdBuffer.m_cmdBuffer, firstBinding, bindingCount, vbos.data(), offsets.data());
}

void CmdBufRecorder::bindPipeline(const std::shared_ptr<GPipelineVLK> &pipeline) {
    if (m_currentPipeline == pipeline) return;

    vkCmdBindPipeline(m_gCmdBuffer.m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipeline());

    m_currentPipeline = pipeline;
}

void CmdBufRecorder::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t firstInstance) {
    vkCmdDrawIndexed(m_gCmdBuffer.m_cmdBuffer,
                     indexCount,
                     instanceCount,
                     firstIndex,
                     0, //m_currentVertexBuffer->getOffset(),
                     firstInstance);
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

void CmdBufRecorder::submitBufferUploads(const std::shared_ptr<GBufferVLK> &bufferVLK) {
    auto submitRecords = bufferVLK->getSubmitRecords();

    if (submitRecords.get().empty())
        return;

    for (auto &submitRecord : submitRecords.get()) {
        vkCmdCopyBuffer(m_gCmdBuffer.m_cmdBuffer,
                        submitRecord.src,
                        submitRecord.dst,
                        submitRecord.copyRegions.size(),
                        submitRecord.copyRegions.data());
    }
}

void CmdBufRecorder::setViewPort(ViewportType viewportType) {
    const constexpr uint32_t firstViewport = 0;
    const constexpr uint32_t viewportCount = 1;

    if (m_currentViewport != viewportType) {
        vkCmdSetViewport(m_gCmdBuffer.m_cmdBuffer, firstViewport, viewportCount, &viewportsForThisStage[(int) viewportType]);
        m_currentViewport = viewportType;
    }
}

void CmdBufRecorder::setScissors(const std::array<int32_t, 2> &areaOffset,
                                 const std::array<uint32_t, 2> &areaSize) {
    const constexpr uint32_t firstScissor = 0;
    const constexpr uint32_t scissorCount = 1;

    VkRect2D vkRect2D = {
        .offset = {
            areaOffset[0],
            areaOffset[1]
        },
        .extent = {
            areaSize[0],
            areaSize[1]
        }
    };

    vkCmdSetScissor(m_gCmdBuffer.m_cmdBuffer, firstScissor, scissorCount, &vkRect2D);

    m_currentScissorsIsDefault = false;
}

void CmdBufRecorder::setDefaultScissors() {
    if (m_currentScissorsIsDefault) return;

    const constexpr uint32_t firstScissor = 0;
    const constexpr uint32_t scissorCount = 1;
    vkCmdSetScissor(m_gCmdBuffer.m_cmdBuffer, firstScissor, scissorCount, &defaultScissor);

    m_currentScissorsIsDefault = true;
}

void CmdBufRecorder::createViewPortTypes(const std::array<int32_t, 2> &areaOffset,
                                         const std::array<uint32_t, 2> &areaSize,
                                         bool invertZ) {
    m_currentViewport = ViewportType::vp_none;

    VkViewport &usualViewport = viewportsForThisStage[(int)ViewportType::vp_usual];
    usualViewport.width = areaSize[0];
    usualViewport.height = areaSize[1];
    usualViewport.x = areaOffset[0];
    usualViewport.y = areaOffset[1];
    if (!invertZ) {
        usualViewport.minDepth = 0;
        usualViewport.maxDepth = 0.990f;
    } else {
        usualViewport.minDepth = 0.06f;
        usualViewport.maxDepth = 1.0f;
    }

    VkViewport &mapAreaViewport = viewportsForThisStage[(int)ViewportType::vp_mapArea];
    mapAreaViewport = usualViewport;
    if (!invertZ) {
        mapAreaViewport.minDepth = 0.991f;
        mapAreaViewport.maxDepth = 0.996f;
    } else {
        mapAreaViewport.minDepth = 0.04f;
        mapAreaViewport.maxDepth = 0.05f;
    }

    VkViewport &skyBoxViewport = viewportsForThisStage[(int)ViewportType::vp_skyBox];
    skyBoxViewport = usualViewport;
    if (!invertZ) {
        skyBoxViewport.minDepth = 0.997f;
        skyBoxViewport.maxDepth = 1.0f;
    } else {
        skyBoxViewport.minDepth = 0;
        skyBoxViewport.maxDepth = 0.03f;
    }
}

void CmdBufRecorder::createDefaultScissors(const std::array<int32_t, 2> &areaOffset,
                                           const std::array<uint32_t, 2> &areaSize) {
    defaultScissor = {};
    defaultScissor.offset = {areaOffset[0], areaOffset[1]};
    defaultScissor.extent = {
        static_cast<uint32_t>(areaSize[0]),
        static_cast<uint32_t>(areaSize[1])
    };
}

#ifdef LINK_TRACY
VkCommandBuffer CmdBufRecorder::getNativeCmdBuffer() { return m_gCmdBuffer.getNativeCmdBuffer(); }
TracyVkCtx const &CmdBufRecorder::getTracyContext() { return m_gCmdBuffer.tracyContext; }
#endif
