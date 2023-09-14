//
// Created by Deamon on 9/12/2023.
//

#include "BufferStagingVLK.h"

BufferStagingVLK::BufferStagingVLK(const HGDeviceVLK &device, int size) : m_device(device) {
    {
        VkBufferCreateInfo vbInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        vbInfo.size = size;
        vbInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        vbInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo stagingAllocInfo = {};
        stagingAllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        stagingAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT; //TODO:

        ERR_GUARD_VULKAN(vmaCreateBuffer(device->getVMAAllocator(), &vbInfo, &stagingAllocInfo,
                                         &m_stagingBuffer,
                                         &m_stagingBufferAlloc,
                                         &m_stagingBufferAllocInfo));
    }
}

BufferStagingVLK::~BufferStagingVLK() {
    auto l_device = m_device;
    auto l_buffer = m_stagingBuffer;
    auto l_bufferAlloc = m_stagingBufferAlloc;
    m_device->addDeallocationRecord(
        [l_buffer, l_device, l_bufferAlloc]() {
            vmaDestroyBuffer(l_device->getVMAAllocator(), l_buffer, l_bufferAlloc);
        }
    );
}

void *BufferStagingVLK::getPointer() {
    return m_stagingBufferAllocInfo.pMappedData;
}
