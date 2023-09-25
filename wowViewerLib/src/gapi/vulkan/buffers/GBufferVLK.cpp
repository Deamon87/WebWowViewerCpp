//
// Created by Deamon on 16.12.22.
//

#include "GBufferVLK.h"
#include "../vk_mem_alloc.h"

GBufferVLK::GBufferVLK(const HGDeviceVLK &device, const char *objName,
                       const std::shared_ptr<GStagingRingBuffer> &ringBuff,
                       VkBufferUsageFlags usageFlags, int maxSize, int alignment) : m_device(device) {
    m_usageFlags = usageFlags;
    m_bufferSize = maxSize;
    m_alignment = alignment;

    m_ringBuff = ringBuff;

    m_objName = objName;

    //Create virtual buffer off this native buffer
    auto allocator = OffsetAllocator::Allocator(m_bufferSize);
    offsetAllocator = std::move(allocator);

    m_gpuBuffer = std::make_shared<BufferGpuVLK>(m_device, maxSize, m_usageFlags, m_objName.c_str());
}

GBufferVLK::~GBufferVLK() {

}



VkResult GBufferVLK::allocateSubBuffer(int sizeInBytes, int fakeSize, OffsetAllocator::Allocation &alloc) {
    std::unique_lock<std::mutex> lock(m_mutex, std::defer_lock);

    bool minAddressStrategy = sizeInBytes < fakeSize && fakeSize > 0;

    if (sizeInBytes == 0) {
        alloc = {.offset = 0, .metadata = OffsetAllocator::Allocation::NO_SPACE};
        return VK_SUCCESS;
    }

    uint32_t allocatingSize = sizeInBytes; //Size in bytes
    if (m_alignment > 0) {
        allocatingSize = allocatingSize + m_alignment;
    }
    lock.lock();
    alloc = this->offsetAllocator.allocate(allocatingSize);
    VkResult result = VK_SUCCESS;
    if (alloc.offset == OffsetAllocator::Allocation::NO_SPACE) {
        result = VK_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    if (minAddressStrategy) {
        if (result == VK_SUCCESS && (alloc.offset+fakeSize) > m_bufferSize) {
            this->offsetAllocator.free(alloc);
            result = VK_ERROR_OUT_OF_DEVICE_MEMORY;
        }
    }
    lock.unlock();
    if (result == VK_SUCCESS && m_alignment > 0) {
        int alignDifference = alloc.offset % m_alignment;
        if (alignDifference > 0) {
            alloc.offset = (alloc.offset + ((m_alignment - alignDifference) % m_alignment));
        }        
    }

    return result;
}

void GBufferVLK::deallocateSubBuffer(const OffsetAllocator::Allocation &alloc, const OffsetAllocator::Allocation &uiaAlloc) {
    //Destruction of this virtualBlock happens only in deallocation queue.
    //So it's safe to assume that even if the buffer's handle been changed by the time VirtualFree happens,
    //the virtualBlock was still not been free'd
    //So there would be no error

    if (alloc.metadata != OffsetAllocator::Allocation::NO_SPACE) {
        auto l_alloc = alloc;
        auto l_uiaAlloc = uiaAlloc;
        auto l_weak = weak_from_this();
        m_device->addDeallocationRecord(
            [l_alloc, l_uiaAlloc, l_weak]() {
                auto shared = l_weak.lock();
                if (shared == nullptr) return;

                std::unique_lock<std::mutex> lock(shared->m_mutex, std::defer_lock);
                lock.lock();
                shared->offsetAllocator.free(l_alloc);
                shared->uiaAllocator.free(l_uiaAlloc);
                lock.unlock();
            });
    }
}

void GBufferVLK::uploadData(const void *data, int length) {
    if (length > m_bufferSize) {
        resize(length);
    }

    void * ptr = allocatePtr(0, length);

    memcpy(ptr, data, length);
}

void *GBufferVLK::allocatePtr(int offset, int length) {
    VkBuffer staging;
    int stage_offset;
    auto *ptr = m_ringBuff->allocateNext(length, staging, stage_offset);

    auto frameIndex = m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;
    {
//        std::unique_lock<std::mutex> lock(m_mutex);
        uploadRegionsPerStaging[frameIndex][staging].push_back({
           .srcOffset = static_cast<VkDeviceSize>(stage_offset),
           .dstOffset = static_cast<VkDeviceSize>(offset),
           .size = static_cast<VkDeviceSize>(length)
       });
    }

    return ptr;
}

void GBufferVLK::resize(int newLength) {
    std::unique_lock<std::mutex> lock(m_mutex, std::defer_lock);
    lock.lock();
    auto oldSize = m_bufferSize;
    offsetAllocator.growSize(newLength - oldSize);
    m_bufferSize = newLength;

    lock.unlock();
}

void GBufferVLK::executeOnChangeForBufAndSubBuf() {
    for (std::list<std::weak_ptr<GSubBufferVLK>>::const_iterator it = currentSubBuffers.begin(); it != currentSubBuffers.end(); ++it){
        auto subBuffer = it->lock();
        if (subBuffer != nullptr) {
            subBuffer->executeOnChange();
        }
    }

    executeOnChange();
}

static inline uint8_t BitScanMSB(uint32_t mask)
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

//fakeSize is used to make sure the subBuffer has enough bytes left till end of main buffer.
//used for allocating data for UBO, when you don't want to suballocate whole size.
//For example if only one bone matrix is used out 220, sizeInBytes will be size of one matrix, while fakeSize is 220 matrices
std::shared_ptr<GBufferVLK::GSubBufferVLK> GBufferVLK::getSubBuffer(int sizeInBytes, int fakeSize) {
    OffsetAllocator::Allocation alloc;

    VkResult res = VK_ERROR_OUT_OF_DEVICE_MEMORY;
    if (sizeInBytes < m_bufferSize) {
        res = allocateSubBuffer(sizeInBytes, fakeSize, alloc);
    }

    if(res == VK_SUCCESS)
    {
        OffsetAllocator::Allocation uiaAlloc;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            uiaAlloc = uiaAllocator.allocate(1);
            if (uiaAlloc.offset == OffsetAllocator::Allocation::NO_SPACE) {
                this->uiaAllocator.growSize(1000);
                uiaAlloc = this->uiaAllocator.allocate(1);
            }
        }

        auto subBuffer = std::make_shared<GSubBufferVLK>(
                                                          shared_from_this(),
                                                          alloc,
                                                          sizeInBytes, fakeSize,
                                                          uiaAlloc);


        {
            std::unique_lock<std::mutex> lock(m_subBufferMutex);
            currentSubBuffers.push_back(subBuffer);
            subBuffer->m_iterator = std::prev(currentSubBuffers.end());
        }
        return subBuffer;
    }
    else
    {
        resize(std::max<int>(m_bufferSize*2, 1 << (BitScanMSB(m_bufferSize + fakeSize)+1)));

        return getSubBuffer(sizeInBytes, fakeSize);
    }
}

void GBufferVLK::deleteSubBuffer(std::list<std::weak_ptr<GSubBufferVLK>>::const_iterator &it,
                                 const OffsetAllocator::Allocation &alloc,
                                 const OffsetAllocator::Allocation &uiaAlloc,
                                 int subBuffersize) {


    if (subBuffersize > 0) {
        deallocateSubBuffer(alloc, uiaAlloc);
    }
    {
        std::unique_lock<std::mutex> lock(m_subBufferMutex);
        currentSubBuffers.erase(it);
    }
}

void GBufferVLK::save(int length) {

}

MutexLockedVector<VulkanCopyCommands> GBufferVLK::getSubmitRecords() {
    {
        std::lock_guard<std::mutex> lock(dataToBeUploadedMtx);

        dataToBeUploaded.clear();
        if (m_gpuBuffer->size() != m_bufferSize) {
            auto newGpuBuf = std::make_shared<BufferGpuVLK>(m_device, m_bufferSize, m_usageFlags, m_objName.c_str());

            auto &copyCmd = dataToBeUploaded.emplace_back();
            copyCmd.src = m_gpuBuffer->getBuffer();
            copyCmd.dst = newGpuBuf->getBuffer();
            copyCmd.needsBarrier = true;

            auto &region = copyCmd.copyRegions.emplace_back();
            region.size = m_gpuBuffer->size();
            region.srcOffset = 0;
            region.dstOffset = 0;

            m_gpuBuffer = newGpuBuf;

            executeOnChangeForBufAndSubBuf();
        }

        auto& stagingRecords = uploadRegionsPerStaging[m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT];
        for (auto &stagingRecord : stagingRecords) {
            auto &intervals = stagingRecord.second;
            std::sort(intervals.begin(), intervals.end(), [](auto &a, auto &b) -> bool {
                return
                    a.srcOffset != b.srcOffset
                    ? a.srcOffset < b.srcOffset
                    : a.size < b.size;
            });

            if (!intervals.empty()) {
                auto currInterval = intervals[0];
                const static auto calcIntervalEnd = [](decltype(currInterval) interval, int aligment) -> size_t {
//                    return aligment > 0 ?
//                           ((interval.srcOffset + interval.size + aligment - 1) / aligment) * aligment :
//                           interval.srcOffset + interval.size;
                    return interval.srcOffset + interval.size;
                };

                size_t currIntervalEnd = calcIntervalEnd(currInterval, m_alignment);

                auto &uploadData = dataToBeUploaded.emplace_back();
                uploadData.src = stagingRecord.first;
                uploadData.dst = m_gpuBuffer->getBuffer();

                uploadData.copyRegions = std::vector<VkBufferCopy>(intervals.begin(), intervals.end());
            }
        }
        stagingRecords.clear();
    }

    return MutexLockedVector<VulkanCopyCommands>(dataToBeUploaded, dataToBeUploadedMtx, true);
}

VkBuffer GBufferVLK::getGPUBuffer() {
    return m_gpuBuffer->getBuffer();
}

//----------------------------------------------------------------
//  SubBuffer thing
//----------------------------------------------------------------

GBufferVLK::GSubBufferVLK::GSubBufferVLK(HGBufferVLK parent,
                                         OffsetAllocator::Allocation alloc,
                                         int size, int fakeSize,
                                         OffsetAllocator::Allocation uiaAlloc) : m_parentBuffer(parent) {
    m_alloc = alloc;
    m_uiaAlloc = uiaAlloc;
    m_size = size;
    m_fakeSize = fakeSize > 0 ? fakeSize : m_size;
}

GBufferVLK::GSubBufferVLK::~GSubBufferVLK() {
    m_parentBuffer->deleteSubBuffer(m_iterator, m_alloc, m_uiaAlloc,  m_size);
}

void GBufferVLK::GSubBufferVLK::uploadData(const void *data, int length) {
    if (length > m_size) {
        std::cerr << "invalid dataSize" << std::endl;
    }

    void * dataPointer = m_parentBuffer->allocatePtr(m_alloc.offset, m_size);
    memcpy(dataPointer, data, length);
}

void *GBufferVLK::GSubBufferVLK::getPointer() {
    if (m_size <= 0) return nullptr;

    void * dataPointer = m_parentBuffer->allocatePtr(m_alloc.offset, m_size);
    return dataPointer;
}

void GBufferVLK::GSubBufferVLK::save(int length) {
    if (length > m_size) {
        std::cerr << "invalid dataSize" << std::endl;
    }
    if (m_size <= 0) return;
}

size_t GBufferVLK::GSubBufferVLK::getSize() {
    return m_fakeSize;
}