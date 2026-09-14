//
// Created by Deamon on 06.02.23.
//

#ifndef AWEBWOWVIEWERCPP_COMMANDBUFFERRECORDER_H
#define AWEBWOWVIEWERCPP_COMMANDBUFFERRECORDER_H

#include <memory>
#include "../../GRenderPassVLK.h"
#include "../../GFrameBufferVLK.h"
#include "../../../../renderer/frame/FrameProfile.h"

class ISimpleMaterialVLK;
class CmdBufRecorder;
class RenderPassHelper;
class GCommandBuffer;
class CommandBufferDebugLabel;
class IMesh;
class ComputePipelineVLK;
typedef std::shared_ptr<ComputePipelineVLK> HComputePipelineVLK;

#include "../CommandBuffer.h"
#include "RenderPassHelper.h"
#include "CommandBufferDebugLabel.h"
#include "../../descriptorSets/GDescriptorSet.h"
#include "../../pipeline/GPipelineVLK.h"
#include "../../materials/ISimpleMaterialVLK.h"

#ifdef LINK_TRACY
#define VkZone(buffRecorder,a) TracyVkZone(buffRecorder.getTracyContext(), buffRecorder.getNativeCmdBuffer(), a)
#endif

class CmdBufRecorder {
public:
    enum class ViewportType: int {vp_none = -1, vp_usual = 0, vp_mapArea = 1, vp_skyBox = 2, vp_MAX = 3};
    friend RenderPassHelper;
    friend CommandBufferDebugLabel;

    // simultaneousUse: set VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT for secondary buffers
    // that are executed more than once (or into multiple primaries) per recording
    CmdBufRecorder(GCommandBuffer &cmdBuffer, const std::shared_ptr<GRenderPassVLK> &renderPass, bool simultaneousUse = false);
    CmdBufRecorder(const CmdBufRecorder&) = delete;
    CmdBufRecorder operator=(const CmdBufRecorder&) = delete;
    ~CmdBufRecorder();

    void setSecondaryCmdRenderArea(const std::array<int32_t, 2> &areaOffset,
                                   const std::array<uint32_t, 2> &areaSize);

    void setGBufferMode(bool value) {
        if (m_gbufferMode != value) {
            m_currentPipeline = nullptr;
            m_gbufferMode = value;
        }
    }

    void setZPrefillMode(bool value) {
        if (m_zprefillMode != value) {
            m_currentPipeline = nullptr;
            m_zprefillMode = value;
        }
    }


    RenderPassHelper beginRenderPass(
        bool isAboutToExecSecondaryCMD,
        const std::shared_ptr<GRenderPassVLK> &renderPassVlk,
        const std::shared_ptr<GFrameBufferVLK> &frameBuffer,
        const std::array<int32_t, 2> &areaOffset,
        const std::array<uint32_t, 2> &areaSize,
        const std::array<float,3> &colorClearColor);

    void bindMaterial(const std::shared_ptr<ISimpleMaterialVLK> &material);
    void bindVertexBindings(const std::shared_ptr<IVertexBufferBindings> &vertexBufferBindings);

    CommandBufferDebugLabel beginDebugLabel(const std::string &labelName, const std::array<float, 4> &colors);

    void bindIndexBuffer(const std::shared_ptr<IBuffer> &bufferVlk);
    void bindVertexBuffers(const std::vector<std::shared_ptr<IBuffer>> &bufferVlk);

    inline void bindPipeline(const std::shared_ptr<GPipelineVLK> &pipeline);
    inline void bindDescriptorSets(VkPipelineBindPoint bindPoint, const std::vector<std::shared_ptr<GDescriptorSet>> &descriptorSets);

    void drawMesh(const HGMesh &mesh, CmdBufRecorder::ViewportType viewportType );
    void drawMeshFromId(GMeshVLK *meshVlk, CmdBufRecorder::ViewportType viewportType);

    void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t firstInstance, uint32_t vertexOffset = 0);
    void drawIndexedIndirect(const std::shared_ptr<IBufferVLK> &buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);

    void bindComputePipeline(const HComputePipelineVLK &pipeline);
    void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

    void pushConstants(VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void *pValues);

    void beginConditionalRendering(const std::shared_ptr<IBufferVLK> &buffer, VkDeviceSize offset, bool inverted = false);
    void endConditionalRendering();

    void executeSecondaryCmdBuffer(const std::shared_ptr<GCommandBuffer> &cmdBuffer);

    void setScissors(const std::array<int32_t, 2> &areaOffset,
                     const std::array<uint32_t, 2> &areaSize);
    void setDefaultScissors();

    void blitImage(VkImage srcImage, VkImageLayout srcLayout, VkImage dstImage, VkImageLayout dstLayout,
                   const VkImageBlit &region, VkFilter filter);
    void recordPipelineImageBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const std::vector<VkImageMemoryBarrier> &imageBarrierData);
    void recordPipelineBufferBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const std::vector<VkBufferMemoryBarrier> &imageBarrierData);
    void copyBufferToImage(VkBuffer buffer, VkImage image, const std::vector<VkBufferImageCopy> &regions);
    void submitBufferUploads(const std::shared_ptr<GBufferVLK> &bufferVLK);
    void submitFullMemoryBarrierPostWrite(const std::shared_ptr<GBufferVLK> &bufferVLK);
    void submitFullMemoryBarrierPreWrite(const std::shared_ptr<GBufferVLK> &bufferVLK);

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
    std::array<GDescriptorSet *, GDescriptorSetLayout::MAX_BINDPOINT_NUMBER> m_currentGraphicsDescriptorSet = {nullptr};
    std::array<GDescriptorSet *, GDescriptorSetLayout::MAX_BINDPOINT_NUMBER> m_currentComputeDescriptorSet = {nullptr};
    std::array<GDescriptorSet *, GDescriptorSetLayout::MAX_BINDPOINT_NUMBER> m_currentRayTracingDescriptorSet = {nullptr};
    bool m_currentScissorsIsDefault = false;
    ViewportType m_currentViewport = ViewportType::vp_none;

    std::shared_ptr<ISimpleMaterialVLK> m_material = nullptr;
    std::shared_ptr<IVertexBufferBindings> m_vertexBufferBindings = nullptr;

    //Viewports
    std::array<VkViewport, (int)ViewportType::vp_MAX> viewportsForThisStage;
    VkRect2D defaultScissor;

    bool m_gbufferMode = false;
    bool m_zprefillMode = false;

    void createViewPortTypes(const std::array<int32_t, 2> &areaOffset,
                             const std::array<uint32_t, 2> &areaSize,
                             bool invertZ);

    void createDefaultScissors(const std::array<int32_t, 2> &areaOffset,
                               const std::array<uint32_t, 2> &areaSize);
};

#endif //AWEBWOWVIEWERCPP_COMMANDBUFFERRECORDER_H
