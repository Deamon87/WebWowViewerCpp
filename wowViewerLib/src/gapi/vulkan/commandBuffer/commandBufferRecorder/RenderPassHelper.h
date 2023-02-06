//
// Created by Deamon on 06.02.23.
//

#ifndef AWEBWOWVIEWERCPP_RENDERPASSHELPER_H
#define AWEBWOWVIEWERCPP_RENDERPASSHELPER_H

#include <memory>
#include "CommandBufferRecorder.h"

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


#endif //AWEBWOWVIEWERCPP_RENDERPASSHELPER_H
