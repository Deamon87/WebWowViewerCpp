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
    enum class ViewportType: int {vp_none = -1, vp_usual = 0, vp_mapArea = 1, vp_skyBox = 2, vp_MAX = 3};
    friend RenderPassHelper;

    CmdBufRecorder(GCommandBuffer &cmdBuffer, const std::shared_ptr<GRenderPassVLK> &renderPass);
    CmdBufRecorder(const CmdBufRecorder&) = delete;
    CmdBufRecorder operator=(const CmdBufRecorder&) = delete;
    ~CmdBufRecorder();

    uint32_t getQueueFamily();

    RenderPassHelper beginRenderPass(
        bool isAboutToExecSecondaryCMD,
        const std::shared_ptr<GRenderPassVLK> &renderPassVlk,
        const std::shared_ptr<GFrameBufferVLK> &frameBuffer,
        const std::array<int32_t, 2> &areaOffset,
        const std::array<uint32_t, 2> &areaSize,
        const std::array<float,3> &colorClearColor, float depthClear);

    void bindIndexBuffer(const std::shared_ptr<IBuffer> &bufferVlk);
    void bindVertexBuffers(const std::vector<std::shared_ptr<IBuffer>> &bufferVlk);
    void bindPipeline(const std::shared_ptr<GPipelineVLK> &pipeline);
    void bindDescriptorSet(uint32_t bindIndex, const std::shared_ptr<GDescriptorSet> &descriptorSet);
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t firstInstance);

    void setScissors(const std::array<int32_t, 2> &areaOffset,
                     const std::array<uint32_t, 2> &areaSize);
    void setDefaultScissors();

    void recordPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const std::vector<VkImageMemoryBarrier> &imageBarrierData);
    void copyBufferToImage(VkBuffer buffer, VkImage image, const std::vector<VkBufferImageCopy> &regions);
    void submitBufferUploads(const std::shared_ptr<GBufferVLK> &bufferVLK);

    friend class RenderPassHelper;

    void setViewPort(ViewportType viewportType);
private:
    const GCommandBuffer &m_gCmdBuffer;

    //States
    std::shared_ptr<GRenderPassVLK> m_currentRenderPass = nullptr;
    std::shared_ptr<GPipelineVLK> m_currentPipeline = nullptr;
    std::shared_ptr<IBufferVLK> m_currentIndexBuffer = nullptr;
    std::array<std::shared_ptr<IBufferVLK>, 2> m_currentVertexBuffers;
    std::array<std::shared_ptr<GDescriptorSet>, GDescriptorSetLayout::MAX_BINDPOINT_NUMBER> m_currentDescriptorSet = {nullptr};
    bool m_currentScissorsIsDefault = false;


    //Viewports
    std::array<VkViewport, (int)ViewportType::vp_MAX> viewportsForThisStage;
    VkRect2D defaultScissor;

    void createViewPortTypes(const std::array<int32_t, 2> &areaOffset,
                             const std::array<uint32_t, 2> &areaSize,
                             bool invertZ);

    void createDefaultScissors(const std::array<int32_t, 2> &areaOffset,
                               const std::array<uint32_t, 2> &areaSize);
};

#endif //AWEBWOWVIEWERCPP_COMMANDBUFFERRECORDER_H
