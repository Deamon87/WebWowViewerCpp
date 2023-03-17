//
// Created by Deamon on 3/17/2023.
//

#ifndef AWEBWOWVIEWERCPP_FFXGLOWPASSVLK_H
#define AWEBWOWVIEWERCPP_FFXGLOWPASSVLK_H

#include <memory>

#include "../../MapSceneRenderer.h"
#include "../../../../gapi/vulkan/GDeviceVulkan.h"
#include "../../materials/IMaterialStructs.h"

class FFXGlowPassVLK {
public:
    FFXGlowPassVLK(HGBufferVLK uboBuffer);
    void assignFFXGlowUBOConsts();
    void createFrameBuffers(const HGDeviceVLK &device, int m_width, int m_height);
private:
    std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> m_ffxGlowVs;
    std::array<std::shared_ptr<IBufferChunk<FXGauss::meshWideBlockPS>>,3> m_ffxGlowPSs;

    std::array<std::shared_ptr<GFrameBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_GlowFrameB1;
    std::array<std::shared_ptr<GFrameBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_GlowFrameB2;
};


#endif //AWEBWOWVIEWERCPP_FFXGLOWPASSVLK_H
