//
// Created by Deamon on 9/21/2023.
//

#ifndef AWEBWOWVIEWERCPP_COMMANDBUFFERRECORDER_INLINE_H
#define AWEBWOWVIEWERCPP_COMMANDBUFFERRECORDER_INLINE_H

#include "CommandBufferRecorder.h"

inline void CmdBufRecorder::bindPipeline(const std::shared_ptr<GPipelineVLK> &pipeline) {
    auto *pPipelineVlk = pipeline.get();
    if (m_currentPipeline == pPipelineVlk) return;

    vkCmdBindPipeline(m_gCmdBuffer.m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipeline());

    m_currentPipeline = pPipelineVlk;
    m_currentPipelineLayout = pipeline->getLayout()->getLayout();
}

inline void CmdBufRecorder::bindDescriptorSets(VkPipelineBindPoint bindPoint, const std::vector<std::shared_ptr<GDescriptorSet>> &descriptorSets) {

        //TODO: bindpoints: VK_PIPELINE_BIND_POINT_GRAPHICS and others
        //Which leads to three separate states for:
        // VK_PIPELINE_BIND_POINT_GRAPHICS, VK_PIPELINE_BIND_POINT_COMPUTE, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR
        // Also, implement "Pipeline Layout Compatibility" thing from spec

    assert(
        (bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) ||
        (bindPoint == VK_PIPELINE_BIND_POINT_COMPUTE) ||
        (bindPoint == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)
    );

    auto &currentDescriptorSet =
        (bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) ? m_currentGraphicsDescriptorSet :
        (bindPoint == VK_PIPELINE_BIND_POINT_COMPUTE) ? m_currentComputeDescriptorSet :
        (bindPoint == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) ? m_currentRayTracingDescriptorSet :
        m_currentGraphicsDescriptorSet;


    uint32_t dynamicOffsets[16];
    uint32_t dynamicOffsetsSize = 0;
    uint32_t *p_dynamicOffset;
    p_dynamicOffset = &dynamicOffsets[0];

    std::array<VkDescriptorSet, 16> descs;
    int descAmount = 0;

    uint32_t bindIndexStart = -1;
    for (int i = 0; i < descriptorSets.size(); i++) {
        auto const pDescriptorSet = descriptorSets[i].get();

        if (currentDescriptorSet[i] != pDescriptorSet) {
            bindIndexStart = bindIndexStart==-1 ? i : bindIndexStart;

            if (currentDescriptorSet[i] != nullptr && currentDescriptorSet[i]->getDescSetLayout() != pDescriptorSet->getDescSetLayout()) {
                //The DS was disturbed due to Pipeline Layout Compatibility
                for (int j = i+1; j < 16; j++)
                    currentDescriptorSet[j] = nullptr;
            }

            auto vkDescSet = pDescriptorSet->getDescSet();
            descs[descAmount++] = vkDescSet;

            uint32_t thisSize = 0;
            pDescriptorSet->getDynamicOffsets(p_dynamicOffset, thisSize);
            p_dynamicOffset+=thisSize;
            dynamicOffsetsSize+=thisSize;

            currentDescriptorSet[i] = pDescriptorSet;
        }
    }

    if (descAmount > 0) {
        vkCmdBindDescriptorSets(m_gCmdBuffer.m_cmdBuffer, bindPoint,
                                m_currentPipelineLayout,
                                bindIndexStart,
                                descAmount, descs.data(),
                                dynamicOffsetsSize, dynamicOffsetsSize > 0 ? dynamicOffsets : nullptr);
    }
}

#endif //AWEBWOWVIEWERCPP_COMMANDBUFFERRECORDER_INLINE_H
