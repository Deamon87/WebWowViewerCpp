//
// Created by Deamon on 16.12.22.
//

#ifndef AWEBWOWVIEWERCPP_GBUFFERVLK_H
#define AWEBWOWVIEWERCPP_GBUFFERVLK_H

#include <memory>

class GDeviceVLK;
typedef std::shared_ptr<GDeviceVLK> HGDeviceVLK;

class GBufferVLK;
typedef std::shared_ptr<GBufferVLK> HGBufferVLK;

#include "../GDeviceVulkan.h"
#include "IBufferVLK.h"
#include "../utils/MutexLockedVector.h"
#include "../bindable/DSBindable.h"

class GBufferVLK : public IBufferVLK, public std::enable_shared_from_this<GBufferVLK> {
    friend class GDeviceVLK;
    class GSubBufferVLK;
public:
    GBufferVLK(const HGDeviceVLK &device, VkBufferUsageFlags usageFlags, int maxSize, int alignment = -1);
    ~GBufferVLK() override;

    //Doesn't make actual upload, only queues it.
    void uploadData(void *, int length) override;
    void subUploadData(void *, int offset, int length) override;
    void uploadFromStaging(int offset, int destOffset, int length);
    void addSubBufferForUpload(const std::weak_ptr<GSubBufferVLK> &buffer);

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

    std::shared_ptr<IBuffer> mutate(int newSize) override;
    void resize(int newLength);
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
        VmaVirtualBlock virtualBlock;
    } currentBuffer;

    std::mutex dataToBeUploadedMtx;
    std::vector<VkBufferCopy> dataToBeUploaded;

    std::mutex m_mutex;
private:
    class GSubBufferVLK : public IBufferVLK, public std::enable_shared_from_this<GSubBufferVLK> {
        friend class GBufferVLK;
    public:
        explicit GSubBufferVLK(HGBufferVLK parent, VmaVirtualAllocation alloc, VkDeviceSize offset,
                               int size, int fakeSize,
                               uint8_t * dataPointer);
        ~GSubBufferVLK() override;
        void uploadData(void *data, int length) override;
        void subUploadData(void *data, int offset, int length) override;
        void *getPointer() override;
        void save(int length) override;
        size_t getSize() override;

        std::shared_ptr<IBuffer> mutate(int newSize) override;

        VkBuffer getGPUBuffer() override {
            return m_parentBuffer->getGPUBuffer();
        }
        size_t getOffset() override {
            return m_offset;
        };
    private:
        HGBufferVLK m_parentBuffer;

        VmaVirtualAllocation m_alloc;
        VkDeviceSize m_offset;
        int m_size;
        int m_fakeSize;
        uint8_t * m_dataPointer = nullptr;
        std::list<std::weak_ptr<GSubBufferVLK>>::const_iterator m_iterator;
    };


    std::list<std::weak_ptr<GBufferVLK::GSubBufferVLK>> currentSubBuffers;
    std::vector<std::weak_ptr<GBufferVLK::GSubBufferVLK>> subBuffersForUpload;

//    uploadCache = {};
public:
    std::shared_ptr<GSubBufferVLK> getSubBuffer(int sizeInBytes, int fakeSize = -1);
    void deleteSubBuffer(std::list<std::weak_ptr<GSubBufferVLK>>::const_iterator &it, VmaVirtualAllocation &alloc, int subBuffersize);
private:
    void createBuffer(BufferInternal &buffer);
    void destroyBuffer(BufferInternal &buffer);

    VkResult allocateSubBuffer(BufferInternal &buffer, int sizeInBytes, int fakeSize, VmaVirtualAllocation &alloc, VkDeviceSize &offset);
    void deallocateSubBuffer(BufferInternal &buffer, VmaVirtualAllocation &alloc);
};


#endif //AWEBWOWVIEWERCPP_GBUFFERVLK_H
