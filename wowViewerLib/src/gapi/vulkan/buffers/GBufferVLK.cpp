//
// Created by Deamon on 16.12.22.
//

#include "GBufferVLK.h"

GBufferVLK::GBufferVLK(const HGDeviceVLK &device, VkBufferUsageFlags usageFlags, int maxSize) : m_device(device) {
    m_usageFlags = usageFlags;
    m_bufferSize = maxSize;

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

VkResult GBufferVLK::allocateSubBuffer(BufferInternal &buffer, int sizeInBytes, VmaVirtualAllocation &alloc, VkDeviceSize &offset) {
    VmaVirtualAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.size = sizeInBytes; //Size in bytes

    return vmaVirtualAllocate(buffer.virtualBlock, &allocCreateInfo, &alloc, &offset);
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
            VkResult res = allocateSubBuffer(newBuffer, subBuffer->m_size, alloc, offset);

            if (res != VK_SUCCESS)  {
                std::cerr << "Could not allocate sub-buffer during resize " << std::endl;
            }

            memcpy((uint8_t *)newBuffer.stagingBufferAllocInfo.pMappedData + offset,
                   (uint8_t *)currentBuffer.stagingBufferAllocInfo.pMappedData + subBuffer->m_offset,
                   subBuffer->m_size
               );

            vmaVirtualFree(currentBuffer.virtualBlock, subBuffer->m_alloc);

            subBuffer->m_offset = offset;
            subBuffer->m_alloc = alloc;
        }
    }

    destroyBuffer(currentBuffer);
    newBuffer = currentBuffer;
}

std::shared_ptr<GBufferVLK::GSubBufferVLK> GBufferVLK::getSubBuffer(int sizeInBytes) {
    VmaVirtualAllocation alloc;
    VkDeviceSize offset;
    VkResult res = allocateSubBuffer(currentBuffer, sizeInBytes, alloc, offset);

    if(res == VK_SUCCESS)
    {
        auto subBuffer = std::make_shared<GSubBufferVLK>(
                                                          shared_from_this(),
                                                         alloc,
                                                         offset, sizeInBytes,
                                                         (uint8_t *)currentBuffer.stagingBufferAllocInfo.pMappedData+offset);
        currentSubBuffers.push_back(subBuffer);
        subBuffer->m_iterator = std::prev(currentSubBuffers.end());
        return subBuffer;
    }
    else
    {
        resize(m_bufferSize*2);
        return getSubBuffer(sizeInBytes);
    }
}

void GBufferVLK::deleteSubBuffer(std::list<std::weak_ptr<GSubBufferVLK>>::const_iterator &it) {
    if(auto subBuffer = it->lock()) {
        deallocateSubBuffer(currentBuffer, subBuffer->m_alloc);
    }

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

//----------------------------------------------------------------
//  SubBuffer thing
//----------------------------------------------------------------

GBufferVLK::GSubBufferVLK::GSubBufferVLK(HGBufferVLK parent,
                                         VmaVirtualAllocation alloc,
                                         VkDeviceSize offset, int size,
                                         uint8_t *dataPointer) : m_parentBuffer(parent) {
    m_alloc = alloc;
    m_offset = offset;
    m_size = size;
    m_dataPointer = dataPointer;
}

GBufferVLK::GSubBufferVLK::~GSubBufferVLK() {
    m_parentBuffer->deleteSubBuffer(m_iterator);
}

void GBufferVLK::GSubBufferVLK::uploadData(void *data, int length) {
    if (length > m_size) {
        std::cerr << "invalid dataSize" << std::endl;
    }

    memcpy(m_dataPointer, data, length);

    m_parentBuffer->uploadFromStaging(m_offset, m_offset, length);
}

void GBufferVLK::GSubBufferVLK::subUploadData(void *data, int offset, int length) {
    if (offset + length > m_size) {
        std::cerr << "invalid dataSize" << std::endl;
    }

    memcpy(m_dataPointer + offset, data, length);

    m_parentBuffer->uploadFromStaging(m_offset+offset, m_offset+offset, length);
}

void *GBufferVLK::GSubBufferVLK::getPointer() {
    return m_dataPointer;
}

void GBufferVLK::GSubBufferVLK::save(int length) {
    if (length > m_size) {
        std::cerr << "invalid dataSize" << std::endl;
    }

    m_parentBuffer->uploadFromStaging(m_offset, m_offset, length);
}

size_t GBufferVLK::GSubBufferVLK::getSize() {
    return m_size;
}
