//
// Created by Deamon on 9/8/2023.
//

#include "GStagingRingBuffer.h"

void * GStagingRingBuffer::allocateNext(int size, VkBuffer &o_staging, int &o_offset) {
    auto frame = m_device->getDrawFrameNumber();
    auto &vec = m_stagingBuffers[frame];

    int startOffset = 0;
    int bufferIndex = 0;

    {
//        std::unique_lock l(m_mutex);
        int currentIndex = ( currentOffset ) / STAGE_BUFFER_SIZE;
        int currentIndexAfter = (currentOffset + size) / STAGE_BUFFER_SIZE;

        if (currentIndex != currentIndexAfter)
            currentOffset = currentIndexAfter * STAGE_BUFFER_SIZE;

        startOffset = currentOffset % STAGE_BUFFER_SIZE;
        bufferIndex = currentIndexAfter;

        if (currentIndexAfter >= vec.size()) {
            vec.emplace_back().staging = std::make_shared<BufferStagingVLK>(m_device, STAGE_BUFFER_SIZE);
        }

        currentOffset += size;
    }

    auto &staging = vec[bufferIndex];
    auto allocatedPtr = ((uint8_t *)staging.cpuBuffer.data()) + startOffset;
    o_staging = staging.staging->getBuffer();
    o_offset = startOffset;

    return allocatedPtr;
}

void GStagingRingBuffer::flushBuffers() {
    auto frame = m_device->getDrawFrameNumber();
    auto &vec = m_stagingBuffers[frame];

    int maxIndex =  ( currentOffset ) / STAGE_BUFFER_SIZE;

    for (int i = 0; i < maxIndex; i++) {
        auto &stagingRec = vec[i];
        memcpy(stagingRec.staging->getPointer(), stagingRec.cpuBuffer.data(), STAGE_BUFFER_SIZE);
    }
    if (currentOffset > 0) {
        auto &stagingRec = vec[maxIndex];
        memcpy(stagingRec.staging->getPointer(), stagingRec.cpuBuffer.data(), currentOffset % STAGE_BUFFER_SIZE);
    }

    int prevMaxIndex =  ( currentOffset ) / STAGE_BUFFER_SIZE;
    currentOffset = 0;

    vec.resize(std::min<int>(prevMaxIndex, vec.size()));
}
