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
class BufferGpuVLK;
class GStagingRingBuffer;
typedef std::shared_ptr<GBufferVLK> HGBufferVLK;

#include "../GDeviceVulkan.h"
#include "IBufferVLK.h"
#include "../utils/MutexLockedVector.h"
#include "../bindable/DSBindable.h"

#include "../../../../../3rdparty/OffsetAllocator/offsetAllocator.hpp"
#include "gpu/BufferGpuVLK.h"
#include "GStagingRingBuffer.h"

struct VulkanCopyCommands {
    VkBuffer src;
    VkBuffer dst;
    std::vector<VkBufferCopy> copyRegions;
};

class GBufferVLK : public IBufferVLK, public std::enable_shared_from_this<GBufferVLK> {
    friend class GDeviceVLK;
    class GSubBufferVLK;
public:
    GBufferVLK(const HGDeviceVLK &device, const char *objName, const std::shared_ptr<GStagingRingBuffer> &ringBuff,
               VkBufferUsageFlags usageFlags, int maxSize, int alignment = -1);
    ~GBufferVLK() override;

    //Doesn't make actual upload, only queues it.
    void uploadData(const void *, int length) override;

    void *getPointer() override { return allocatePtr(0, m_bufferSize);}
    //Submits data edited with Pointer
    void save(int length) override;

    size_t getSize() override { return m_bufferSize;};

    VkBuffer getGPUBuffer() override;
    size_t getOffset() override {
        return 0;
    };
    size_t getIndex() override {
        return 0;
    };

    MutexLockedVector<VulkanCopyCommands> getSubmitRecords();

    void resize(int newLength);

    struct uploadInterval {size_t start; size_t size;};

private:
    HGDeviceVLK m_device;
    std::shared_ptr<GStagingRingBuffer> m_ringBuff;

    std::string m_objName;

    VkBufferUsageFlags m_usageFlags;
    int m_bufferSize;
    int m_alignment;

    //Buffers
    std::array<std::unordered_map<VkBuffer, std::vector<VkBufferCopy>>, IDevice::MAX_FRAMES_IN_FLIGHT> uploadRegionsPerStaging;
    std::mutex dataToBeUploadedMtx;
    std::vector<VulkanCopyCommands> dataToBeUploaded;

    OffsetAllocator::Allocator offsetAllocator = OffsetAllocator::Allocator(1000);

    std::mutex m_mutex;
    std::mutex m_subBufferMutex;

    //If this variable is not equal current size -> whole buffer needs to be submitted
    size_t lastSubmittedBufferSize = 0;

private:
    class GSubBufferVLK : public IBufferVLK, public std::enable_shared_from_this<GSubBufferVLK> {
        friend class GBufferVLK;
    public:
        explicit GSubBufferVLK(HGBufferVLK parent, OffsetAllocator::Allocation alloc,
                               int size, int fakeSize,
                               OffsetAllocator::Allocation uiaAlloc);
        ~GSubBufferVLK() override;
        void uploadData(const void *data, int length) override;
        void *getPointer() final override;
        void save(int length) override;
        size_t getSize() override;

        VkBuffer getGPUBuffer() override {
            return m_parentBuffer->getGPUBuffer();
        }
        size_t getOffset() override {
            return m_alloc.offset;
        };
        size_t getIndex() override {
            return (m_parentBuffer->m_alignment > 0) ?
                    m_alloc.offset / m_parentBuffer->m_alignment :
                    m_alloc.offset;
        };
    private:
        HGBufferVLK m_parentBuffer;

        OffsetAllocator::Allocation m_alloc;
        OffsetAllocator::Allocation m_uiaAlloc;
        int m_size;
        int m_fakeSize;

        std::list<std::weak_ptr<GSubBufferVLK>>::const_iterator m_iterator;
    };

    std::shared_ptr<BufferGpuVLK> m_gpuBuffer;

    std::list<std::weak_ptr<GBufferVLK::GSubBufferVLK>> currentSubBuffers;

    OffsetAllocator::Allocator uiaAllocator = OffsetAllocator::Allocator(0);

//    uploadCache = {};
public:
    std::shared_ptr<GSubBufferVLK> getSubBuffer(int sizeInBytes, int fakeSize = -1);
    void deleteSubBuffer(std::list<std::weak_ptr<GSubBufferVLK>>::const_iterator &it,
                         const OffsetAllocator::Allocation &alloc,
                         const OffsetAllocator::Allocation &uiaAlloc,
                         int subBuffersize);
private:
    VkResult allocateSubBuffer(int sizeInBytes, int fakeSize, OffsetAllocator::Allocation &alloc);
    void deallocateSubBuffer(const OffsetAllocator::Allocation &alloc, const OffsetAllocator::Allocation &uiaAlloc);

    void executeOnChangeForBufAndSubBuf();

    void* allocatePtr(int offset, int length);
};


#endif //AWEBWOWVIEWERCPP_GBUFFERVLK_H
