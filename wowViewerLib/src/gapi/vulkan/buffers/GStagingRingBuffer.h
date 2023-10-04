//
// Created by Deamon on 9/8/2023.
//

#ifndef AWEBWOWVIEWERCPP_GSTAGINGRINGBUFFER_H
#define AWEBWOWVIEWERCPP_GSTAGINGRINGBUFFER_H

class BufferStagingVLK;

#include <array>
#include "../GDeviceVulkan.h"
#include "gpu/BufferStagingVLK.h"

class GStagingRingBuffer {
    static constexpr uint32_t STAGE_BUFFER_SIZE = 20*1024*1024;
public:
    GStagingRingBuffer(const HGDeviceVLK &device) : m_device(device) {};

    void* allocateNext(int size, VkBuffer &o_staging, int &o_offset);

    void flushBuffers();
private:
    HGDeviceVLK m_device;

    std::mutex m_mutex;

    std::array<std::atomic<uint64_t>, IDevice::MAX_FRAMES_IN_FLIGHT> offsets = {0};
    struct BufferAndCPU {
        std::shared_ptr<BufferStagingVLK> staging;
        std::array<uint8_t, STAGE_BUFFER_SIZE> cpuBuffer;
    };

    std::array<std::vector<std::shared_ptr<BufferAndCPU>>, IDevice::MAX_FRAMES_IN_FLIGHT> m_stagingBuffers;
};


#endif //AWEBWOWVIEWERCPP_GSTAGINGRINGBUFFER_H
