//
// Created by Deamon on 16.12.22.
//

#ifndef AWEBWOWVIEWERCPP_GBUFFERVLK_H
#define AWEBWOWVIEWERCPP_GBUFFERVLK_H

#include <memory>
#include <atomic>

class GDeviceVLK;
typedef std::shared_ptr<GDeviceVLK> HGDeviceVLK;

class GBufferVLK;
typedef std::shared_ptr<GBufferVLK> HGBufferVLK;

#include "../GDeviceVulkan.h"
#include "IBufferVLK.h"
#include "../utils/MutexLockedVector.h"
#include "../bindable/DSBindable.h"

#include "../../../../../3rdparty/OffsetAllocator/offsetAllocator.hpp"

class GBufferVLK : public IBufferVLK, public std::enable_shared_from_this<GBufferVLK> {
    friend class GDeviceVLK;
    class GSubBufferVLK;
public:
    GBufferVLK(const HGDeviceVLK &device, VkBufferUsageFlags usageFlags, int maxSize, int alignment = -1);
    ~GBufferVLK() override;

    //Doesn't make actual upload, only queues it.
    void uploadData(const void *, int length) override;
    void uploadFromStaging(int offset, int destOffset, int length);
    void addIntervalIndexForUpload(int index);

    void *getPointer() override { return currentBuffer.stagingBufferAllocInfo.pMappedData;};
    //Submits data edited with Pointer
    void save(int length) override;


    size_t getSize() override { return m_bufferSize;};

    VkBuffer getGPUBuffer() override {
        return currentBuffer.g_hBuffer;
    }
    VkBuffer getCPUBuffer() {
        return currentBuffer.stagingBuffer;
    }
    size_t getOffset() override {
        return 0;
    };

    MutexLockedVector<VkBufferCopy> getSubmitRecords();

    void resize(int newLength);

    struct uploadInterval {size_t start; size_t size;};
    struct uploadIntervalActivatable : uploadInterval {
        bool requiresUpdate = false;
    };
private:
    HGDeviceVLK m_device;

    VkBufferUsageFlags m_usageFlags;
    int m_bufferSize;
    int m_alignment;
    //Buffers
    struct BufferInternal {
        VkBuffer g_hBuffer = VK_NULL_HANDLE;
        VmaAllocation g_hBufferAlloc = VK_NULL_HANDLE;

        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        VmaAllocation stagingBufferAlloc = VK_NULL_HANDLE;
        VmaAllocationInfo stagingBufferAllocInfo;

        //Virtual block for suballocations
    } currentBuffer;

    std::mutex dataToBeUploadedMtx;
    std::vector<VkBufferCopy> dataToBeUploaded;

    OffsetAllocator::Allocator offsetAllocator = OffsetAllocator::Allocator(1000);

    std::mutex m_mutex;

    //If this variable is not equal current size -> whole buffer needs to be submitted
    size_t lastSubmittedBufferSize = 0;

private:
    class GSubBufferVLK : public IBufferVLK, public std::enable_shared_from_this<GSubBufferVLK> {
        friend class GBufferVLK;
    public:
        explicit GSubBufferVLK(HGBufferVLK parent, OffsetAllocator::Allocation alloc,
                               int size, int fakeSize,
                               OffsetAllocator::Allocation uiaAlloc,
                               uint8_t * dataPointer);
        ~GSubBufferVLK() override;
        void uploadData(const void *data, int length) override;
        void *getPointer() override;
        void save(int length) override;
        size_t getSize() override;

        VkBuffer getGPUBuffer() override {
            return m_parentBuffer->getGPUBuffer();
        }
        size_t getOffset() override {
            return m_alloc.offset;
        };
    private:
        void setParentDataPointer(void * ptr) {
            m_dataPointer = ((uint8_t *) ptr) + m_alloc.offset;
        }
    private:
        HGBufferVLK m_parentBuffer;

        OffsetAllocator::Allocation m_alloc;
        OffsetAllocator::Allocation m_uiaAlloc;
        int m_size;
        int m_fakeSize;
        uint8_t * m_dataPointer = nullptr;
        std::list<std::weak_ptr<GSubBufferVLK>>::const_iterator m_iterator;
    };



    std::list<std::weak_ptr<GBufferVLK::GSubBufferVLK>> currentSubBuffers;
    std::vector<uploadInterval> uploadIntervals;

    OffsetAllocator::Allocator uiaAllocator = OffsetAllocator::Allocator(0);
    std::vector<uploadIntervalActivatable> uploadIntervalActivatable;

//    uploadCache = {};
public:
    std::shared_ptr<GSubBufferVLK> getSubBuffer(int sizeInBytes, int fakeSize = -1);
    void deleteSubBuffer(std::list<std::weak_ptr<GSubBufferVLK>>::const_iterator &it,
                         const OffsetAllocator::Allocation &alloc,
                         const OffsetAllocator::Allocation &uiaAlloc,
                         int subBuffersize);
private:
    void createBuffer(BufferInternal &buffer);
    void destroyBuffer(BufferInternal &buffer);

    VkResult allocateSubBuffer(BufferInternal &buffer, int sizeInBytes, int fakeSize, OffsetAllocator::Allocation &alloc);
    void deallocateSubBuffer(BufferInternal &buffer, const OffsetAllocator::Allocation &alloc, const OffsetAllocator::Allocation &uiaAlloc);
};


#endif //AWEBWOWVIEWERCPP_GBUFFERVLK_H
