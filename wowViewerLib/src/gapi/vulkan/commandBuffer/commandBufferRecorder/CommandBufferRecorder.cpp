//
// Created by Deamon on 06.02.23.
//

#include "CommandBufferRecorder.h"
#include "CommandBufferRecorder_inline.h"
#include "../../pipeline/GPipelineVLK.h"

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
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT |
        ((renderPass != nullptr) ? (VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) : 0);
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
        std::cerr << "failed to record command buffer!" << std::endl;
        throw std::runtime_error("failed to record command buffer!");
    }
}

void CmdBufRecorder::setSecondaryCmdRenderArea(const std::array<int32_t, 2> &areaOffset,
                                   const std::array<uint32_t, 2> &areaSize) {
    std::array<uint32_t, 2> fixedAreasSize = {
        std::max<uint32_t>(1, areaSize[0]),
        std::max<uint32_t>(1, areaSize[1])
    };

    createViewPortTypes(areaOffset, fixedAreasSize, m_currentRenderPass->getInvertZ());
    createDefaultScissors(areaOffset, fixedAreasSize);

    setDefaultScissors();
}

RenderPassHelper CmdBufRecorder::beginRenderPass(
    bool isAboutToExecSecondaryCMD,
    const std::shared_ptr<GRenderPassVLK> &renderPassVlk,
    const std::shared_ptr<GFrameBufferVLK> &frameBuffer,
    const std::array<int32_t, 2> &areaOffset,
    const std::array<uint32_t, 2> &areaSize,
    const std::array<float,3> &colorClearColor
) {
    if (m_currentRenderPass != nullptr) {
        throw std::runtime_error("tried to start render pass with another pass being active already");
    }

    std::array<uint32_t, 2> fixedAreasSize = {
        std::max<uint32_t>(1, areaSize[0]),
        std::max<uint32_t>(1, areaSize[1])
    };

    createViewPortTypes(areaOffset, fixedAreasSize, renderPassVlk->getInvertZ());
    createDefaultScissors(areaOffset, fixedAreasSize);

    m_currentRenderPass = renderPassVlk;

    m_currentPipeline = nullptr;
    m_currentPipelineLayout = nullptr;
    m_currentIndexBuffer = nullptr;
    m_currentVertexBuffers = {};
    m_currentDescriptorSet = {};
    m_material = nullptr;
    m_vertexBufferBindings = nullptr;

    auto renderPass = RenderPassHelper(
        *this,
        isAboutToExecSecondaryCMD,
        renderPassVlk, frameBuffer, areaOffset, fixedAreasSize, colorClearColor
    );
    if (!isAboutToExecSecondaryCMD) {
        setDefaultScissors();
    }
    return renderPass;
}


CommandBufferDebugLabel CmdBufRecorder::beginDebugLabel(const std::string &labelName, const std::array<float, 4> &colors) {
    return CommandBufferDebugLabel(
        *this,
        labelName,
        colors
    );
}

void CmdBufRecorder::bindIndexBuffer(const std::shared_ptr<IBuffer> &buffer) {
    if (m_currentIndexBuffer == buffer) return;

    auto bufferVlk = std::dynamic_pointer_cast<IBufferVLK>(buffer);

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
        //firstBinding == i <- means we can only skip the start of the list;
        if (firstBinding == i && m_currentVertexBuffers[i] == buffers[i]) {
            firstBinding++;
            continue;
        }

        auto const &bufferVlk = std::dynamic_pointer_cast<IBufferVLK>(buffers[i]);
        auto pbufferVlk = bufferVlk.get();

        vbos[vboCnt++] = pbufferVlk->getGPUBuffer();
        offsets[offsetCnt++] = pbufferVlk->getOffset();

        m_currentVertexBuffers[i] = bufferVlk;
    }

    int bindingCount = vboCnt;
    if (vboCnt == 0) return;

    vkCmdBindVertexBuffers(m_gCmdBuffer.m_cmdBuffer, firstBinding, bindingCount, vbos.data(), offsets.data());
}



void CmdBufRecorder::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t firstInstance, uint32_t vertexOffset) {
    vkCmdDrawIndexed(m_gCmdBuffer.m_cmdBuffer,
                     indexCount,
                     instanceCount,
                     firstIndex,
                     vertexOffset,
                     firstInstance);
}

void CmdBufRecorder::executeSecondaryCmdBuffer(const std::shared_ptr<GCommandBuffer> &cmdBuffer) {
    std::array<VkCommandBuffer, 1> buffers = {cmdBuffer->getNativeCmdBuffer()};

    vkCmdExecuteCommands(m_gCmdBuffer.m_cmdBuffer, 1, buffers.data());
}

void CmdBufRecorder::recordPipelineImageBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
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

void CmdBufRecorder::recordPipelineBufferBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                 const std::vector<VkBufferMemoryBarrier> &bufferBarrierData) {
    vkCmdPipelineBarrier(
        m_gCmdBuffer.m_cmdBuffer,
        srcStageMask,
        dstStageMask,
        0,
        0, nullptr,
        bufferBarrierData.size(), bufferBarrierData.data(),
        0, nullptr);
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
//    TracyMessageStr(("Submiting buffer " + bufferVLK->getName()));
    ZoneScopedN("submitBufferUploads");


    auto submitRecords = bufferVLK->getSubmitRecords();

    if (submitRecords.get().empty())
        return;

    std::vector<VkBufferMemoryBarrier> barrierVec = {{}};

    const auto &submits = submitRecords.get();
    for (int i = 0; i < submits.size(); i++) {
        auto &submit = submits[i];
        if (submit.needsBarrier && !submit.copyRegions.empty()) {
            barrierVec[0] = {
                VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                nullptr,
                0,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                submit.src,
                submit.copyRegions[0].srcOffset,
                submit.copyRegions[0].size
            };
            this->recordPipelineBufferBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrierVec);
        }

        vkCmdCopyBuffer(m_gCmdBuffer.m_cmdBuffer,
                        submit.src,
                        submit.dst,
                        submit.copyRegions.size(),
                        submit.copyRegions.data());

        if (submit.needsBarrier && !submit.copyRegions.empty()) {
            barrierVec[0] =  {
                VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                nullptr,
                VK_ACCESS_NONE,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                submit.dst,
                submit.copyRegions[0].dstOffset,
                submit.copyRegions[0].size
            };
            this->recordPipelineBufferBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrierVec);
        }
    }
}
void CmdBufRecorder::submitFullMemoryBarrierPostWrite(const std::shared_ptr<GBufferVLK> &bufferVLK) {
    std::vector<VkBufferMemoryBarrier> barrierVec = {
        {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            nullptr,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            bufferVLK->getGPUBuffer(),
            0,
            bufferVLK->getGPUBufferSize()
        }
    };

    this->recordPipelineBufferBarrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, barrierVec);
}
void CmdBufRecorder::submitFullMemoryBarrierPreWrite(const std::shared_ptr<GBufferVLK> &bufferVLK) {
    std::vector<VkBufferMemoryBarrier> barrierVec = {
        {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            nullptr,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            bufferVLK->getGPUBuffer(),
            0,
            bufferVLK->getGPUBufferSize()
        }
    };

    this->recordPipelineBufferBarrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, barrierVec);
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

    VkOffset2D offset2D = {areaOffset[0], areaOffset[1]};
    VkExtent2D extent2D = {
        static_cast<uint32_t>(areaSize[0]),
        static_cast<uint32_t>(areaSize[1])
    };

    if (
        defaultScissor.offset.x != offset2D.x || defaultScissor.offset.y != offset2D.y ||
        defaultScissor.extent.width != extent2D.width || defaultScissor.extent.height != extent2D.height
    ) {
        defaultScissor = {};
        defaultScissor.offset = offset2D;
        defaultScissor.extent = extent2D;

        m_currentScissorsIsDefault = false;
    }
}

void CmdBufRecorder::bindMaterial(const std::shared_ptr<ISimpleMaterialVLK> &material) {
    if (m_material == material) return;

    //1. Bind pipeline
    if (m_gbufferMode) {
        this->bindPipeline(material->getGBufferPipeline());
    } else {
        this->bindPipeline(material->getPipeline());
    }

    //2. Bind Descriptor sets
    auto const &descSets = material->getDescriptorSets();
    this->bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, descSets);

    m_material = material;
}

void CmdBufRecorder::bindVertexBindings(const std::shared_ptr<IVertexBufferBindings> &vertexBufferBindings) {
    if (m_vertexBufferBindings == vertexBufferBindings) return;

    auto vulkanBindings = std::dynamic_pointer_cast<GVertexBufferBindingsVLK>(vertexBufferBindings);

    //1. Bind VBOs
    this->bindVertexBuffers(vulkanBindings->getVertexBuffers());

    //2. Bind IBOs
    this->bindIndexBuffer(vulkanBindings->getIndexBuffer());

    m_vertexBufferBindings = vertexBufferBindings;
}

#ifdef LINK_TRACY
VkCommandBuffer CmdBufRecorder::getNativeCmdBuffer() { return m_gCmdBuffer.getNativeCmdBuffer(); }
TracyVkCtx const& CmdBufRecorder::getTracyContext() { return m_gCmdBuffer.tracyContext; }
#endif