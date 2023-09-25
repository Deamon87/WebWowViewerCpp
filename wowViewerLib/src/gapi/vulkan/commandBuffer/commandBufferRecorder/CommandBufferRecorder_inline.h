//
// Created by Deamon on 9/21/2023.
//

#ifndef AWEBWOWVIEWERCPP_COMMANDBUFFERRECORDER_INLINE_H
#define AWEBWOWVIEWERCPP_COMMANDBUFFERRECORDER_INLINE_H

#include "CommandBufferRecorder.h"

inline void CmdBufRecorder::bindPipeline(const std::shared_ptr<GPipelineVLK> &pipeline) {
    auto *pPipelineVlk = pipeline.get();
    if (m_currentPipeline == pPipelineVlk) return;

    vkCmdBindPipeline(m_gCmdBuffer.m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipelineVlk->getPipeline());

    m_currentPipeline = pPipelineVlk;
    m_currentPipelineLayout = pipeline->getLayout();
}

inline void CmdBufRecorder::bindDescriptorSet(VkPipelineBindPoint bindPoint, uint32_t bindIndex, const std::shared_ptr<GDescriptorSet> &descriptorSet) {
    {
        //TODO: bindpoints: VK_PIPELINE_BIND_POINT_GRAPHICS and others
        //Which leads to three separate states for:
        // VK_PIPELINE_BIND_POINT_GRAPHICS, VK_PIPELINE_BIND_POINT_COMPUTE, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR
        // Also, implement "Pipeline Layout Compatibility" thing from spec

//    if (m_currentDescriptorSet[bindIndex] == descriptorSet) return;

        auto pDescriptorSet = descriptorSet.get();

        if (m_currentDescriptorSet[bindIndex] != pDescriptorSet) {
            auto vkDescSet = pDescriptorSet->getDescSet();
            constexpr uint32_t vkDescCnt = 1;


            uint32_t dynamicOffsets[16];
            uint32_t dynamicOffsetsSize;
            pDescriptorSet->getDynamicOffsets(dynamicOffsets, dynamicOffsetsSize);

            vkCmdBindDescriptorSets(m_gCmdBuffer.m_cmdBuffer, bindPoint,
                                    m_currentPipelineLayout,
                                    bindIndex,
                                    vkDescCnt, &vkDescSet,
                                    dynamicOffsetsSize, dynamicOffsetsSize > 0 ? dynamicOffsets : nullptr);

            m_currentDescriptorSet[bindIndex] = pDescriptorSet;
        }
    }
}

#endif //AWEBWOWVIEWERCPP_COMMANDBUFFERRECORDER_INLINE_H
