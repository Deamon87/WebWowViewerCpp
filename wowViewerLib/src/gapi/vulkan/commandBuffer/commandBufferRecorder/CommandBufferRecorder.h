//
// Created by Deamon on 06.02.23.
//

#ifndef AWEBWOWVIEWERCPP_COMMANDBUFFERRECORDER_H
#define AWEBWOWVIEWERCPP_COMMANDBUFFERRECORDER_H

#include <memory>
#include "../../GRenderPassVLK.h"
#include "../../GFrameBufferVLK.h"
#include "../../../../renderer/frame/FrameProfile.h"



class CmdBufRecorder;
class RenderPassHelper;
class GCommandBuffer;
class CommandBufferDebugLabel;

#include "../CommandBuffer.h"
#include "RenderPassHelper.h"
#include "CommandBufferDebugLabel.h"
#include "../../descriptorSets/GDescriptorSet.h"
#include "../../GPipelineVLK.h"

#ifdef LINK_TRACY
#define VkZone(buffRecorder,a) TracyVkZone(buffRecorder.getTracyContext(), buffRecorder.getNativeCmdBuffer(), a)
#endif

class CmdBufRecorder {
public:
    enum class ViewportType: int {vp_none = -1, vp_usual = 0, vp_mapArea = 1, vp_skyBox = 2, vp_MAX = 3};
    friend RenderPassHelper;
    friend CommandBufferDebugLabel;

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

    CommandBufferDebugLabel beginDebugLabel(const std::string &labelName, const std::array<float, 4> &colors);

    void bindIndexBuffer(const std::shared_ptr<IBuffer> &bufferVlk);
    void bindVertexBuffers(const std::vector<std::shared_ptr<IBuffer>> &bufferVlk);

    inline void bindPipeline(const std::shared_ptr<GPipelineVLK> &pipeline);
    inline void bindDescriptorSets(VkPipelineBindPoint bindPoint, const std::vector<std::shared_ptr<GDescriptorSet>> &descriptorSets);

    void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t firstInstance);

    void setScissors(const std::array<int32_t, 2> &areaOffset,
                     const std::array<uint32_t, 2> &areaSize);
    void setDefaultScissors();

    void recordPipelineImageBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const std::vector<VkImageMemoryBarrier> &imageBarrierData);
    void recordPipelineBufferBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const std::vector<VkBufferMemoryBarrier> &imageBarrierData);
    void copyBufferToImage(VkBuffer buffer, VkImage image, const std::vector<VkBufferImageCopy> &regions);
    void submitBufferUploads(const std::shared_ptr<GBufferVLK> &bufferVLK);

    friend class RenderPassHelper;

    void setViewPort(ViewportType viewportType);

#ifdef LINK_TRACY
    //Those should only be used for initialization of tracy zone
    TracyVkCtx const &getTracyContext();;
    VkCommandBuffer getNativeCmdBuffer();;
#endif
private:
    const GCommandBuffer &m_gCmdBuffer;

    static constexpr int MAX_VERTEX_BUFFERS_PER_DRAWCALL = 2;

    //States
    std::shared_ptr<GRenderPassVLK> m_currentRenderPass = nullptr;
    GPipelineVLK *m_currentPipeline = nullptr;
    VkPipelineLayout m_currentPipelineLayout = nullptr;
    std::shared_ptr<IBufferVLK> m_currentIndexBuffer = nullptr;
    std::array<std::shared_ptr<IBufferVLK>, MAX_VERTEX_BUFFERS_PER_DRAWCALL> m_currentVertexBuffers;
    std::array<GDescriptorSet *, GDescriptorSetLayout::MAX_BINDPOINT_NUMBER> m_currentDescriptorSet = {nullptr};
    bool m_currentScissorsIsDefault = false;
    ViewportType m_currentViewport = ViewportType::vp_none;

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
