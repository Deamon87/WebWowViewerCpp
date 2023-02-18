//
// Created by Deamon on 06.02.23.
//

#ifndef AWEBWOWVIEWERCPP_COMMANDBUFFERRECORDER_H
#define AWEBWOWVIEWERCPP_COMMANDBUFFERRECORDER_H

#include <memory>
#include "../../GRenderPassVLK.h"
#include "../../GFrameBufferVLK.h"



class CmdBufRecorder;
class RenderPassHelper;
class GCommandBuffer;

#include "../CommandBuffer.h"
#include "RenderPassHelper.h"
#include "../../descriptorSets/GDescriptorSet.h"

class CmdBufRecorder {
public:
    friend RenderPassHelper;

    CmdBufRecorder(GCommandBuffer &cmdBuffer, const std::shared_ptr<GRenderPassVLK> &renderPass);
    ~CmdBufRecorder();

    uint32_t getQueueFamily();

    RenderPassHelper beginRenderPass(
        bool isAboutToExecSecondaryCMD,
        const std::shared_ptr<GRenderPassVLK> &renderPassVlk,
        const std::shared_ptr<GFrameBufferVLK> &frameBuffer,
        const std::array<int32_t, 2> &areaOffset,
        const std::array<uint32_t, 2> &areaSize,
        const std::array<float,3> &colorClearColor, float depthClear);

    void bindIndexBuffer(std::shared_ptr<GBufferVLK> &bufferVlk);
    void bindVertexBuffer(std::shared_ptr<GBufferVLK> &bufferVlk);
    void bindPipeline(std::shared_ptr<GPipelineVLK> &pipeline);
    void bindDescriptorSet(uint32_t bindIndex, const std::shared_ptr<GDescriptorSet> &descriptorSet);
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t firstInstance);


    void recordPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const std::vector<VkImageMemoryBarrier> &imageBarrierData);
    void copyBufferToImage(VkBuffer buffer, VkImage image, const std::vector<VkBufferImageCopy> &regions);
    void submitBufferUploads(const std::shared_ptr<GBufferVLK> &bufferVLK);

    friend class RenderPassHelper;
private:
    const GCommandBuffer &m_gCmdBuffer;

    //States
    std::shared_ptr<GRenderPassVLK> m_currentRenderPass = nullptr;
    std::shared_ptr<GPipelineVLK> m_currentPipeline = nullptr;
    std::shared_ptr<GBufferVLK> m_currentIndexBuffer = nullptr;
    std::shared_ptr<GBufferVLK> m_currentVertexBuffer = nullptr;
    std::array<std::shared_ptr<GDescriptorSet>, GDescriptorSetLayout::MAX_BINDPOINT_NUMBER> m_currentDescriptorSet = {nullptr};
};

#endif //AWEBWOWVIEWERCPP_COMMANDBUFFERRECORDER_H
