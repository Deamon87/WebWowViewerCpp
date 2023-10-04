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

    //Add alignment
    size += 16;

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
                std::unique_lock l(m_mutex);//

                while (currentIndexAfter >= vec.size()) {
                    auto &bufferAndCPU = vec.emplace_back();
                    bufferAndCPU = std::make_shared<BufferAndCPU>();
                    bufferAndCPU->staging = std::make_shared<BufferStagingVLK>(m_device, STAGE_BUFFER_SIZE);
                }
            }

        } while(currentIndex != currentIndexAfter);

    bufferIndex = currentIndex;
    startOffset = offset % STAGE_BUFFER_SIZE;
    if (bufferIndex >= vec.size()) {
        throw "OOOOSP";
    }
    auto &bufferAndCPU = vec[bufferIndex];
    auto buffPtr = reinterpret_cast<intptr_t>(bufferAndCPU->cpuBuffer.data());

    uint32_t buffPtrAlign = buffPtr % 16;
    uint32_t offsetAlign = startOffset % 16;
    uint32_t alignAdd = 0;
    if (buffPtrAlign > offsetAlign) {
        alignAdd = (16-buffPtrAlign) + (buffPtrAlign - offsetAlign);
    } else {
        alignAdd = (16-buffPtrAlign) + ((16 - offsetAlign) + buffPtrAlign);
    }
    alignAdd %= 16;
    startOffset += alignAdd;

    auto allocatedPtr = ((uint8_t *)bufferAndCPU->cpuBuffer.data()) + startOffset;
    if ((startOffset + size) > bufferAndCPU->cpuBuffer.size()) {
        throw "OOOOSP";
    }
    o_staging = bufferAndCPU->staging->getBuffer();
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
        memcpy(stagingRec->staging->getPointer(), stagingRec->cpuBuffer.data(), STAGE_BUFFER_SIZE);
    }
    if (currentOffset > 0) {
        auto &stagingRec = vec[maxIndex];
        memcpy(stagingRec->staging->getPointer(), stagingRec->cpuBuffer.data(), currentOffset % STAGE_BUFFER_SIZE);
    }

    uint32_t prevMaxIndex =  ( currentOffset ) / STAGE_BUFFER_SIZE;
    offsets[frame] = 0;

    vec.resize(std::min<int>(prevMaxIndex+1, vec.size()));
}
