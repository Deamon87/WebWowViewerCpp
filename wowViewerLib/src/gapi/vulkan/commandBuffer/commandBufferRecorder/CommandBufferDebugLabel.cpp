//
// Created by Deamon on 5/16/2023.
//

#include "CommandBufferDebugLabel.h"
#include "CommandBufferRecorder.h"

CommandBufferDebugLabel::CommandBufferDebugLabel(CmdBufRecorder &cmdBufRecorder, const std::string &labelStr,
                                                 const std::array<float, 4> &colors) : m_cmdBufRecorder(cmdBufRecorder) {
    if (vkCmdBeginDebugUtilsLabelEXT != nullptr) {
        VkDebugUtilsLabelEXT label;
        label.sType =  VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        label.pNext = NULL;
        label.pLabelName = labelStr.c_str();
        label.color[0] = colors[0];
        label.color[1] = colors[1];
        label.color[2] = colors[2];
        label.color[3] = colors[3];

        auto &gCmdBuffer = m_cmdBufRecorder.m_gCmdBuffer;

        vkCmdBeginDebugUtilsLabelEXT(gCmdBuffer.getNativeCmdBuffer(), &label);
    }
}

CommandBufferDebugLabel::~CommandBufferDebugLabel() {
    if (vkCmdEndDebugUtilsLabelEXT != nullptr) {
        auto &gCmdBuffer = m_cmdBufRecorder.m_gCmdBuffer;
        vkCmdEndDebugUtilsLabelEXT(gCmdBuffer.getNativeCmdBuffer());
    }
}
