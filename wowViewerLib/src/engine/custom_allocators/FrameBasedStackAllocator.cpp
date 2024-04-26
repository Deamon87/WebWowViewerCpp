//
// Created by Deamon on 12/13/2023.
//

#include "FrameBasedStackAllocator.h"
#include <atomic>
#include <vector>
#include <array>

#include "../../gapi/interface/IDevice.h"
#include <memory>
#include <list>

static inline uint8_t BitScanMSB2(uint32_t mask)
{
#ifdef _MSC_VER
    unsigned long pos;
    if (_BitScanReverse(&pos, mask))
        return static_cast<uint8_t>(pos);
#elif defined __GNUC__ || defined __clang__
    if (mask)
        return 31 - static_cast<uint8_t>(__builtin_clz(mask));
#else
    uint8_t pos = 31;
    uint32_t bit = 1UL << 31;
    do
    {
        if (mask & bit)
            return pos;
        bit >>= 1;
    } while (pos-- > 0);
#endif
    return UINT8_MAX;
}

struct MemoryStoringPool {
    MemoryStoringPool(size_t size) {
        currentOffset = 0;
        data.resize(size);
    }
    int currentOffset;
    std::vector<uint8_t, tbb::cache_aligned_allocator<uint8_t>> data;
};



std::array<std::mutex, 2*IDevice::MAX_FRAMES_IN_FLIGHT> g_frameStackMutexes;
std::array<std::list<std::unique_ptr<MemoryStoringPool>>, 2*IDevice::MAX_FRAMES_IN_FLIGHT> g_frameStackList;

inline void * allocateFromMemPool(size_t alignedSize, std::unique_ptr<MemoryStoringPool> &frameRegion, int &regionSize) {
    void *ptr = nullptr;

    regionSize = frameRegion->data.size();

    bool isWithinBoundary = (frameRegion->currentOffset + ((alignedSize+16)) < regionSize);
    if (isWithinBoundary) {
        ptr = &frameRegion->data[frameRegion->currentOffset];
        frameRegion->currentOffset += alignedSize;
    }

    return ptr;
}

void * frameLinearAllocate(size_t size) {
    auto currentFrame = IDevice::getCurrentProcessingFrameNumber() % (2 * IDevice::MAX_FRAMES_IN_FLIGHT);

    std::lock_guard<std::mutex> lock(g_frameStackMutexes[currentFrame]);
    auto &current_list = g_frameStackList[currentFrame];

    size_t alignedSize = (size + 16) & (~15L);

    void *ptr = nullptr;
    int regionSize = 2048;
    if (current_list.size() > 0) {
        auto &memPoolRegion = current_list.front();

        if (memPoolRegion) {
            ptr = allocateFromMemPool(alignedSize, memPoolRegion, regionSize);
        }
    }
    if (ptr == nullptr) {
        size_t currentBufferSize = regionSize;
        size_t newSize = std::max<int>(currentBufferSize * 2, 1 << (BitScanMSB2(currentBufferSize + size) + 1));

        auto newPool = std::make_unique<MemoryStoringPool>(newSize);
        ptr = allocateFromMemPool(alignedSize, newPool, regionSize);

        current_list.push_front(std::move(newPool));
    }
    if (ptr == nullptr) {
        std::cout << "oops!!!" << std::endl;
    }

    return ptr;
}

void allocatorBeginFrame(unsigned int frameNum) {
    auto currentFrame = frameNum % (2 * IDevice::MAX_FRAMES_IN_FLIGHT);
    std::lock_guard<std::mutex> lock(g_frameStackMutexes[currentFrame]);

    if (!g_frameStackList[currentFrame].empty()) {
        g_frameStackList[currentFrame].resize(1);
        g_frameStackList[currentFrame].front()->currentOffset = 0;
    }
}