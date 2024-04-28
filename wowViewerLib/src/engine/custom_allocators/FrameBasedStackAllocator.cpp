//
// Created by Deamon on 12/13/2023.
//

#include "FrameBasedStackAllocator.h"
#include <atomic>
#include <vector>
#include <array>

#include "../../gapi/interface/IDevice.h"
#include "../../../3rdparty/OffsetAllocator/offsetAllocator.hpp"
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

    MemoryStoringPool(size_t size) : m_Allocator(OffsetAllocator::Allocator(size)) {

        data.resize(size);
    }
    bool isInThisRegion(void *p) {
        bool biggerThanLowerBound = (((uintptr_t)p) > ((uintptr_t)data.data()));
        bool lessThanHigherBound = ((((uintptr_t)p) - ((uintptr_t)data.data())) < data.size());

        return biggerThanLowerBound && lessThanHigherBound;
    }

    inline void * allocateFromMemPool(size_t size, int &regionSize) {
        void *ptr = nullptr;

        regionSize = data.size();

        OffsetAllocator::Allocation alloc = m_Allocator.allocate(size+16+sizeof(OffsetAllocator::Allocation));
        if (alloc.offset == OffsetAllocator::Allocation::NO_SPACE) {
            return nullptr;
        }

        ptr = &data[alloc.offset & (~15)];
        *(OffsetAllocator::Allocation *) &(((uint8_t*) ptr)[size]) = alloc;

        return ptr;
    }
    void deallocate(void *ptr, size_t size) {
        if (!this->isInThisRegion(ptr)) {
            return;
        }

        OffsetAllocator::Allocation alloc = *(OffsetAllocator::Allocation *) &(((uint8_t*) ptr)[size]);
        m_Allocator.free(alloc);
    }

    private:
        OffsetAllocator::Allocator m_Allocator;
        std::vector<uint8_t, tbb::cache_aligned_allocator<uint8_t>> data;
};



std::array<std::mutex, 2*IDevice::MAX_FRAMES_IN_FLIGHT> g_frameStackMutexes;
std::mutex g_g_frameStackMutexes;
std::array<std::list<std::unique_ptr<MemoryStoringPool>>, 2*IDevice::MAX_FRAMES_IN_FLIGHT> g_frameStackList;



void * frameLinearAllocate(size_t size) {
    auto currentFrame = IDevice::getCurrentProcessingFrameNumber() % (2 * IDevice::MAX_FRAMES_IN_FLIGHT);

    std::lock_guard<std::mutex> g_lock(g_g_frameStackMutexes);
    std::lock_guard<std::mutex> lock(g_frameStackMutexes[currentFrame]);
    auto &current_list = g_frameStackList[currentFrame];

//    size_t alignedSize = (size + 16) & (~15L);

    void *ptr = nullptr;
    int regionSize = 2048;
    if (current_list.size() > 0) {
        auto &memPoolRegion = current_list.front();

        if (memPoolRegion) {
            ptr = memPoolRegion->allocateFromMemPool(size, regionSize);
        }
    }
    if (ptr == nullptr) {
        size_t currentBufferSize = regionSize;
        size_t newSize = std::max<int>(currentBufferSize * 2, 1 << (BitScanMSB2(currentBufferSize + size) + 1));

        auto newPool = std::make_unique<MemoryStoringPool>(newSize);
        ptr = newPool->allocateFromMemPool(size, regionSize);

        current_list.push_front(std::move(newPool));
    }
    if (ptr == nullptr) {
        std::cout << "oops!!!" << std::endl;
    }

    return ptr;
}

void frameDeAllocate(void *ptr, std::size_t n) {
    auto currentFrame = IDevice::getCurrentProcessingFrameNumber() % (2 * IDevice::MAX_FRAMES_IN_FLIGHT);

    std::lock_guard<std::mutex> g_lock(g_g_frameStackMutexes);

    for (auto &frameList : g_frameStackList) {
        for (auto it = frameList.begin(); it != frameList.end(); ++it) {
            auto &region = (*it);
            if (region->isInThisRegion(ptr)) {
                region->deallocate(ptr, n);

                return;
            }
        }
    }

//    std::cout << "failed to deallocate" << std::endl;
};

void allocatorBeginFrame(unsigned int frameNum) {
    auto currentFrame = frameNum % (2 * IDevice::MAX_FRAMES_IN_FLIGHT);
    std::lock_guard<std::mutex> lock(g_frameStackMutexes[currentFrame]);

    if (!g_frameStackList[currentFrame].empty()) {
        g_frameStackList[currentFrame].resize(1);
    }
}

