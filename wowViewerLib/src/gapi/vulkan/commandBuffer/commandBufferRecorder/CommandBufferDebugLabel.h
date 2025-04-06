//
// Created by Deamon on 5/16/2023.
//

#ifndef AWEBWOWVIEWERCPP_COMMANDBUFFERDEBUGLABEL_H
#define AWEBWOWVIEWERCPP_COMMANDBUFFERDEBUGLABEL_H

#include "CommandBufferRecorder.h"

class CommandBufferDebugLabel {
public:
    CommandBufferDebugLabel(CmdBufRecorder &cmdBufRecorder, const std::string &labelStr, const std::array<float, 4> &colors);

    CommandBufferDebugLabel(const CommandBufferDebugLabel&) = delete;
    CommandBufferDebugLabel operator=(const CommandBufferDebugLabel&) = delete;

    ~CommandBufferDebugLabel();
private:
    CmdBufRecorder &m_cmdBufRecorder;
};


#endif //AWEBWOWVIEWERCPP_COMMANDBUFFERDEBUGLABEL_H
