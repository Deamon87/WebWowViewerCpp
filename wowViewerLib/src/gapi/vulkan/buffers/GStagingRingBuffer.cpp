//
// Created by Deamon on 9/8/2023.
//

#include "GStagingRingBuffer.h"

void * GStagingRingBuffer::allocateNext(int size, VkBuffer &o_staging, int &o_offset) {
    auto frame = m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;
    auto &vec = m_stagingBuffers[frame];

    int startOffset = 0;
    int bufferIndex = 0;
    auto &currentOffset = offsets[frame];

    if (size > STAGE_BUFFER_SIZE)
        throw std::runtime_error(("size > STAGE_BUFFER_SIZE; size = " + std::to_string(size)));


        int currentIndex = 0;
        int currentIndexAfter = 0;
        uint64_t offset = 0;
        do {
            offset = atomic_fetch_add(&currentOffset, size);

            currentIndex = ( offset ) / STAGE_BUFFER_SIZE;
            currentIndexAfter = (offset + size) / STAGE_BUFFER_SIZE;

            if (currentIndexAfter >= vec.size()) {
                std::unique_lock l(m_mutex);
                while (currentIndexAfter >= vec.size()) {
                    vec.emplace_back().staging = std::make_shared<BufferStagingVLK>(m_device, STAGE_BUFFER_SIZE);
                }
            }

        } while(currentIndex != currentIndexAfter);

    bufferIndex = currentIndex;
    startOffset = offset % STAGE_BUFFER_SIZE;

    auto &staging = vec[bufferIndex];
    auto allocatedPtr = ((uint8_t *)staging.cpuBuffer.data()) + startOffset;
    if ((startOffset + size) > staging.cpuBuffer.size()) {
        size++;
    }
    o_staging = staging.staging->getBuffer();
    o_offset = startOffset;

    return allocatedPtr;
}

void GStagingRingBuffer::flushBuffers() {
    auto frame = m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;
    auto &vec = m_stagingBuffers[frame];

    uint64_t currentOffset = offsets[frame];

    uint32_t maxIndex =  ( currentOffset ) / STAGE_BUFFER_SIZE;

    for (int i = 0; i < maxIndex; i++) {
        auto &stagingRec = vec[i];
        memcpy(stagingRec.staging->getPointer(), stagingRec.cpuBuffer.data(), STAGE_BUFFER_SIZE);
    }
    if (currentOffset > 0) {
        auto &stagingRec = vec[maxIndex];
        memcpy(stagingRec.staging->getPointer(), stagingRec.cpuBuffer.data(), currentOffset % STAGE_BUFFER_SIZE);
    }

    uint32_t prevMaxIndex =  ( currentOffset ) / STAGE_BUFFER_SIZE;
    offsets[frame] = 0;

    vec.resize(std::min<int>(prevMaxIndex+1, vec.size()));
}
