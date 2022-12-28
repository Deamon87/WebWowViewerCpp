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

class GBufferVLK : public IBufferVLK, public std::enable_shared_from_this<GBufferVLK> {
    friend class GDeviceVLK;
public:
    explicit GBufferVLK(const HGDeviceVLK &device, VkBufferUsageFlags usageFlags, int maxSize);
    ~GBufferVLK() override;

    //Doesn't make actual upload, only queues it.
    void uploadData(void *, int length) override;
    void subUploadData(void *, int offset, int length) override;
    void uploadFromStaging(int offset, int destOffset, int length);

    void *getPointer() override { return currentBuffer.stagingBufferAllocInfo.pMappedData;};
    //Submits data edited with Pointer
    void save(int length) override;


    size_t getSize() override { return m_bufferSize;};

    VkBuffer getGPUBuffer() override {
        return currentBuffer.g_hBuffer;
    }
    size_t getOffset() override {
        return 0;
    };

    void resize(int newLength);
private:
    HGDeviceVLK m_device;

    VkBufferUsageFlags m_usageFlags;
    int m_bufferSize;
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

    std::vector<VkBufferCopy> dataToBeUploaded;
private:
    class GSubBufferVLK : public IBufferVLK {
        friend class GBufferVLK;
    public:
        explicit GSubBufferVLK(HGBufferVLK parent, VmaVirtualAllocation alloc, VkDeviceSize offset, int size, uint8_t * dataPointer);
        ~GSubBufferVLK() override;
        void uploadData(void *data, int length) override;
        void subUploadData(void *data, int offset, int length) override;
        void *getPointer() override;
        void save(int length) override;
        size_t getSize() override;

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
        uint8_t * m_dataPointer = nullptr;
        std::list<std::weak_ptr<GSubBufferVLK>>::const_iterator m_iterator;
    };


    std::list<std::weak_ptr<GBufferVLK::GSubBufferVLK>> currentSubBuffers;
public:
    std::shared_ptr<GSubBufferVLK> getSubBuffer(int sizeInBytes);
    void deleteSubBuffer(std::list<std::weak_ptr<GSubBufferVLK>>::const_iterator &it);
private:
    void createBuffer(BufferInternal &buffer);
    void destroyBuffer(BufferInternal &buffer);

    VkResult allocateSubBuffer(BufferInternal &buffer, int sizeInBytes, VmaVirtualAllocation &alloc, VkDeviceSize &offset);
    void deallocateSubBuffer(BufferInternal &buffer, VmaVirtualAllocation &alloc);
};


#endif //AWEBWOWVIEWERCPP_GBUFFERVLK_H
