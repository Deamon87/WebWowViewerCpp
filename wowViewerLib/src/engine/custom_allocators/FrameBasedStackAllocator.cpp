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

class FrameRegion {
public:
    FrameRegion(size_t size) : m_buffer(size) {

    }
    inline int currentAllocationNumber() { return m_currentAllocations; }

    void *allocate(size_t allocSize) {
        int offset = atomic_fetch_add(&m_topIndex, allocSize);
        if ((offset + allocSize) > m_buffer.size()) {
            return nullptr;
        }
        m_currentAllocations++;

        return &m_buffer[offset];
    }
    int getCurrentSize() { return m_buffer.size();}

    bool isThisRegion(void *ptr) {
        int index = (uintptr_t)ptr - (uintptr_t)m_buffer.data();

        return (index > 0) && (index < m_buffer.size());
    }
    void deallocate(void *ptr) {
        if (!this->isThisRegion(ptr)) {
            return;
        }

        m_currentAllocations--;

        int index = ((uintptr_t)ptr - (uintptr_t)m_buffer.data());
        int prev_value = m_topIndex;
        while(prev_value > index &&
              !m_topIndex.compare_exchange_weak(prev_value, index))
        {}
    }

private:
    std::vector<uint8_t> m_buffer;
    std::atomic<int> m_topIndex = 0;
    std::atomic<int> m_currentAllocations = 0;
};


std::array<std::list<std::unique_ptr<FrameRegion>>, IDevice::MAX_FRAMES_IN_FLIGHT> g_frameStackList;
//std::array<std::mutex, IDevice::MAX_FRAMES_IN_FLIGHT> frames;

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

void * frameAllocate(size_t size) {
    auto currentFrame = IDevice::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;
    auto &current_list = g_frameStackList[currentFrame];

    auto &frameRegion = current_list.back();
    void *ptr = nullptr;
    int regionSize = 2048;
    if (frameRegion) {
        ptr = frameRegion->allocate(size);
        regionSize = frameRegion->getCurrentSize();
    }
    if (ptr == nullptr) {
        size_t currentBufferSize = regionSize;
        size_t newSize = std::max<int>(currentBufferSize * 2, 1 << (BitScanMSB2(currentBufferSize + size) + 1));

        current_list.push_back(std::make_unique<FrameRegion>(newSize));

        ptr = current_list.back()->allocate(size);
    }
    if (ptr == nullptr) {
        std::cout << "oops!!!" << std::endl;
    }

    return ptr;
}
void frameDeAllocate(void *ptr) {
//    auto currentFrame = IDevice::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;
//    auto &current_list = frames[currentFrame];

    for (auto &frameList : g_frameStackList) {
        for (auto it = frameList.begin(); it != frameList.end(); ++it) {
            auto &region = (*it);
            if (region->isThisRegion(ptr)) {
                region->deallocate(ptr);

                if (region->currentAllocationNumber() == 0)
                    frameList.erase(it);

                return;
            }
        }
    }

    std::cout << "failed to deallocate" << std::endl;
};