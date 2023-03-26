//
// Created by Deamon on 16.12.22.
//

#include "GBufferVLK.h"
#include "../vk_mem_alloc.h"

GBufferVLK::GBufferVLK(const HGDeviceVLK &device, VkBufferUsageFlags usageFlags, int maxSize, int alignment) : m_device(device) {
    m_usageFlags = usageFlags;
    m_bufferSize = maxSize;
    m_alignment = alignment;

    createBuffer(currentBuffer);
}

GBufferVLK::~GBufferVLK() {
    destroyBuffer(currentBuffer);
}

void GBufferVLK::createBuffer(BufferInternal &buffer) {
//Create new buffer for VBO
    VkBufferCreateInfo vbInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    vbInfo.size = m_bufferSize;
    vbInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vbInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo ibAllocCreateInfo = {};
    ibAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    ibAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    ERR_GUARD_VULKAN(vmaCreateBuffer(m_device->getVMAAllocator(), &vbInfo, &ibAllocCreateInfo,
                                     &buffer.stagingBuffer,
                                     &buffer.stagingBufferAlloc,
                                     &buffer.stagingBufferAllocInfo));

    // No need to flush stagingBuffer memory because CPU_ONLY memory is always HOST_COHERENT.
    vbInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | m_usageFlags;
    ibAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    ibAllocCreateInfo.flags = 0;
    ERR_GUARD_VULKAN(vmaCreateBuffer(m_device->getVMAAllocator(), &vbInfo, &ibAllocCreateInfo,
                                     &buffer.g_hBuffer,
                                     &buffer.g_hBufferAlloc, nullptr));

    //Create virtual buffer off this native buffer
    VmaVirtualBlockCreateInfo blockCreateInfo = {};
    blockCreateInfo.size = m_bufferSize;

    VkResult res = vmaCreateVirtualBlock(&blockCreateInfo, &buffer.virtualBlock);
    if(res != VK_SUCCESS)
    {
        std::cerr << "Failed to create virtual buffer" << std::endl;
    }

}

void GBufferVLK::destroyBuffer(BufferInternal &buffer) {
    auto l_device = m_device;
    auto l_buffer = buffer;
    m_device->addDeallocationRecord(
        [l_buffer, l_device]() {
            vmaClearVirtualBlock(l_buffer.virtualBlock);
            vmaDestroyVirtualBlock(l_buffer.virtualBlock);

            vmaDestroyBuffer(l_device->getVMAAllocator(), l_buffer.stagingBuffer, l_buffer.stagingBufferAlloc);
            vmaDestroyBuffer(l_device->getVMAAllocator(), l_buffer.g_hBuffer, l_buffer.g_hBufferAlloc);
        }
    );
}

VkResult GBufferVLK::allocateSubBuffer(BufferInternal &buffer, int sizeInBytes, int fakeSize, VmaVirtualAllocation &alloc, VkDeviceSize &offset) {
    bool minAddressStrategy = sizeInBytes < fakeSize && fakeSize > 0;

    VmaVirtualAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.size = sizeInBytes; //Size in bytes
    if (minAddressStrategy) {
        allocCreateInfo.flags = VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_OFFSET_BIT;
    }
    if (m_alignment > 0) {
        allocCreateInfo.alignment = m_alignment;
    }

    auto result = vmaVirtualAllocate(buffer.virtualBlock, &allocCreateInfo, &alloc, &offset);
    if (minAddressStrategy) {
        if (result == VK_SUCCESS && (offset+fakeSize) > m_bufferSize) {
            vmaVirtualFree(buffer.virtualBlock, alloc);
            result = VK_ERROR_OUT_OF_DEVICE_MEMORY;
        }
    }
    return result;
}

void GBufferVLK::deallocateSubBuffer(BufferInternal &buffer, VmaVirtualAllocation &alloc) {
    vmaVirtualFree(buffer.virtualBlock, alloc);
}

void GBufferVLK::uploadData(void *data, int length) {
    if (length > m_bufferSize) {
        resize(length);
    }

    memcpy(currentBuffer.stagingBufferAllocInfo.pMappedData, data, length);

    uploadFromStaging(0, 0, length);
}

void GBufferVLK::subUploadData(void *data, int offset, int length) {
    if (offset+length > m_bufferSize) {
        resize(offset+length);
    }
    memcpy((uint8_t *)currentBuffer.stagingBufferAllocInfo.pMappedData+offset, data, length);
}

std::shared_ptr<IBuffer> GBufferVLK::mutate(int newSize) {
    resize(newSize);
    return shared_from_this();
}
void GBufferVLK::resize(int newLength) {
    m_bufferSize = newLength;

    BufferInternal newBuffer;
    createBuffer(newBuffer);

    //Reallocate subBuffers and copy their data
    for (std::list<std::weak_ptr<GSubBufferVLK>>::const_iterator it = currentSubBuffers.begin(); it != currentSubBuffers.end(); ++it){
        auto subBuffer = it->lock();
        if (subBuffer != nullptr) {
            VmaVirtualAllocation alloc;
            VkDeviceSize offset;
            VkResult res = allocateSubBuffer(newBuffer, subBuffer->m_size, subBuffer->m_fakeSize, alloc, offset);

            if (res != VK_SUCCESS)  {
                std::cerr << "Could not allocate sub-buffer during resize " << std::endl;
            }

            memcpy((uint8_t *)newBuffer.stagingBufferAllocInfo.pMappedData + offset,
                   (uint8_t *)currentBuffer.stagingBufferAllocInfo.pMappedData + subBuffer->m_offset,
                   subBuffer->m_size
               );

            deallocateSubBuffer(currentBuffer, subBuffer->m_alloc);

            addSubBufferForUpload(subBuffer);

            subBuffer->m_offset = offset;
            subBuffer->m_alloc = alloc;
            subBuffer->m_dataPointer = (uint8_t *)newBuffer.stagingBufferAllocInfo.pMappedData+offset;
        }
    }

    destroyBuffer(currentBuffer);
    currentBuffer = newBuffer;

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
    VmaVirtualAllocation alloc;
    VkDeviceSize offset;

    std::unique_lock<std::mutex> lock(m_mutex,std::defer_lock);
    lock.lock();
    VkResult res = allocateSubBuffer(currentBuffer, sizeInBytes, fakeSize, alloc, offset);
    lock.unlock();

    if(res == VK_SUCCESS)
    {
        auto subBuffer = std::make_shared<GSubBufferVLK>(
                                                          shared_from_this(),
                                                         alloc,
                                                         offset, sizeInBytes, fakeSize,
                                                         (uint8_t *)currentBuffer.stagingBufferAllocInfo.pMappedData+offset);

        lock.lock();
        currentSubBuffers.push_back(subBuffer);
        subBuffer->m_iterator = std::prev(currentSubBuffers.end());
        lock.unlock();
        return subBuffer;
    }
    else
    {
        lock.lock();
        resize(std::max<int>(m_bufferSize*2, BitScanMSB(m_bufferSize + fakeSize) << 1));
        lock.unlock();
        return getSubBuffer(sizeInBytes, fakeSize);
    }
}

void GBufferVLK::deleteSubBuffer(std::list<std::weak_ptr<GSubBufferVLK>>::const_iterator &it, VmaVirtualAllocation &m_alloc) {
    deallocateSubBuffer(currentBuffer, m_alloc);
    
    currentSubBuffers.erase(it);
}

void GBufferVLK::uploadFromStaging(int offset, int destOffset, int length) {
    std::lock_guard<std::mutex> lock(dataToBeUploadedMtx);

    VkBufferCopy &vbCopyRegion = dataToBeUploaded.emplace_back();
    vbCopyRegion.srcOffset = offset;
    vbCopyRegion.dstOffset = destOffset;
    vbCopyRegion.size = length;
}

void GBufferVLK::save(int length) {
    uploadFromStaging(0, 0, length);
}

void GBufferVLK::addSubBufferForUpload(const std::shared_ptr<IBufferVLK> &buffer) {
    std::lock_guard<std::mutex> lock(dataToBeUploadedMtx);

    subBuffersForUpload.emplace_back(buffer);
}

MutexLockedVector<VkBufferCopy> GBufferVLK::getSubmitRecords() {
    {
        std::lock_guard<std::mutex> lock(dataToBeUploadedMtx);

        struct uploadInterval {size_t start; size_t size;};
        std::vector<uploadInterval> intervals;
        for (const auto &subBuffer : subBuffersForUpload) {
            auto &newInterval = intervals.emplace_back();
            newInterval = {.start = subBuffer->getOffset(), .size = subBuffer->getSize()};
        }

        std::sort(intervals.begin(), intervals.end(), [](auto &a, auto &b ) -> bool {
            return a.start < b.start;
        });

        if (!intervals.empty()) {
            auto currInterval = intervals[0];
            const static auto calcIntervalEnd = [](decltype(currInterval) interval, int aligment) -> size_t {
                return aligment > 0 ?
                    ((interval.start + interval.size + aligment - 1) / aligment) * aligment:
                    interval.start + interval.size;
            };

            size_t currIntervalEnd = calcIntervalEnd(currInterval, m_alignment);

            for (int i = 1; i < intervals.size(); i++) {
                auto &nextInterval = intervals[i];
                if (currIntervalEnd < nextInterval.start) {
                    VkBufferCopy &vbCopyRegion = dataToBeUploaded.emplace_back();
                    vbCopyRegion.srcOffset = currInterval.start;
                    vbCopyRegion.dstOffset = currInterval.start;
                    vbCopyRegion.size = currInterval.size;

                    currInterval = intervals[i];
                    currIntervalEnd = calcIntervalEnd(currInterval, m_alignment);
                } else {
                    assert(nextInterval.start - currInterval.start >= 0);
                    currInterval.size = (nextInterval.start - currInterval.start) + nextInterval.size;
                    currIntervalEnd = calcIntervalEnd(currInterval, m_alignment);
                }
            }

            VkBufferCopy &vbCopyRegion = dataToBeUploaded.emplace_back();
            vbCopyRegion.srcOffset = currInterval.start;
            vbCopyRegion.dstOffset = currInterval.start;
            vbCopyRegion.size = currInterval.size;
        }
        subBuffersForUpload.clear();
    }


    return MutexLockedVector<VkBufferCopy>(dataToBeUploaded, dataToBeUploadedMtx, true);
}

//----------------------------------------------------------------
//  SubBuffer thing
//----------------------------------------------------------------

GBufferVLK::GSubBufferVLK::GSubBufferVLK(HGBufferVLK parent,
                                         VmaVirtualAllocation alloc,
                                         VkDeviceSize offset,
                                         int size, int fakeSize,
                                         uint8_t *dataPointer) : m_parentBuffer(parent) {
    m_alloc = alloc;
    m_offset = offset;
    m_size = size;
    m_fakeSize = fakeSize > 0 ? fakeSize : m_size;
    m_dataPointer = dataPointer;
}

GBufferVLK::GSubBufferVLK::~GSubBufferVLK() {
    m_parentBuffer->deleteSubBuffer(m_iterator, m_alloc);
}

void GBufferVLK::GSubBufferVLK::uploadData(void *data, int length) {
    if (length > m_size) {
        std::cerr << "invalid dataSize" << std::endl;
    }

    memcpy(m_dataPointer, data, length);

    m_parentBuffer->addSubBufferForUpload(m_iterator->lock());
//    m_parentBuffer->uploadFromStaging(m_offset, m_offset, length);
}

void GBufferVLK::GSubBufferVLK::subUploadData(void *data, int offset, int length) {
    if (offset + length > m_size) {
        std::cerr << "invalid dataSize" << std::endl;
    }

    memcpy(m_dataPointer + offset, data, length);

    m_parentBuffer->addSubBufferForUpload(m_iterator->lock());
}

void *GBufferVLK::GSubBufferVLK::getPointer() {
    return m_dataPointer;
}

void GBufferVLK::GSubBufferVLK::save(int length) {
    if (length > m_size) {
        std::cerr << "invalid dataSize" << std::endl;
    }

    m_parentBuffer->addSubBufferForUpload(m_iterator->lock());
//    m_parentBuffer->uploadFromStaging(m_offset, m_offset, length);
}

size_t GBufferVLK::GSubBufferVLK::getSize() {
    return m_fakeSize;
}
std::shared_ptr<IBuffer> GBufferVLK::GSubBufferVLK::mutate(int newSize) {
    return m_parentBuffer->getSubBuffer(newSize);
};
