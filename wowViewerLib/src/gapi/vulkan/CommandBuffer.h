//
// Created by Deamon on 05.02.23.
//

#ifndef AWEBWOWVIEWERCPP_COMMANDBUFFER_H
#define AWEBWOWVIEWERCPP_COMMANDBUFFER_H


#include "../vulkan/volk.h"
#include "GDeviceVulkan.h"
#include "GFrameBufferVLK.h"

class GCommandBuffer {
public:
    class CmdBufRecorder {
    public:

        class RenderPassHelper {
        public:
            explicit RenderPassHelper(CmdBufRecorder &cmdBufRecorder,
                                      bool isAboutToExecSecondaryCMD,
                                      const std::shared_ptr<GRenderPassVLK> &renderPassVlk,
                                      const std::shared_ptr<GFrameBufferVLK> &frameBuffer,
                                      const std::array<int32_t, 2> &areaOffset,
                                      const std::array<uint32_t, 2> &areaSize,
                                      const std::array<float,3> &colorClearColor, float depthClear);

            ~RenderPassHelper();
        private:
            CmdBufRecorder &m_cmdBufRecorder;
        };

    public:
        CmdBufRecorder(GCommandBuffer &cmdBuffer, const std::shared_ptr<GRenderPassVLK> &renderPass);
        ~CmdBufRecorder();

        RenderPassHelper beginRenderPass(
                                         bool isAboutToExecSecondaryCMD,
                                         const std::shared_ptr<GRenderPassVLK> &renderPassVlk,
                                         const std::shared_ptr<GFrameBufferVLK> &frameBuffer,
                                         const std::array<int32_t, 2> areaOffset,
                                         const std::array<uint32_t, 2> areaSize,
                                         const std::array<float,3> &colorClearColor, float depthClear);

        void bindPipeline(std::shared_ptr<GPipelineVLK> &pipeline);
        void bindDescriptorSet(uint32_t bindIndex, std::shared_ptr<GDescriptorSet> &descriptorSet);

        friend RenderPassHelper::~RenderPassHelper();
    private:
        const GCommandBuffer &m_gCmdBuffer;

        //States
        std::shared_ptr<GRenderPassVLK> m_currentRenderPass = nullptr;
        std::shared_ptr<GPipelineVLK> m_currentPipeline = nullptr;
        std::array<std::shared_ptr<GDescriptorSet>, GDescriptorSet::MAX_BINDPOINT_NUMBER> m_currentDescriptorSet = {nullptr};
    };

public:
    GCommandBuffer(GDeviceVLK &deviceVlk, VkCommandPool commandPool, bool isPrimary);

    CmdBufRecorder beginRecord(const std::shared_ptr<GRenderPassVLK> &renderPass);
private:
    VkCommandBuffer m_cmdBuffer;
    bool m_isPrimary = true;
};


#endif //AWEBWOWVIEWERCPP_COMMANDBUFFER_H
