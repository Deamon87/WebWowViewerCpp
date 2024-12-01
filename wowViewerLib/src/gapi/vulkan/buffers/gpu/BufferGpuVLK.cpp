//
// Created by Deamon on 9/12/2023.
//

#include "BufferGpuVLK.h"

BufferGpuVLK::BufferGpuVLK(const HGDeviceVLK &device, int size, VkBufferUsageFlags usageFlags, const char *obj_name) : m_device(device) {
    m_size = size;

    VmaAllocationInfo allocationInfo;

    VkBufferCreateInfo vbInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    vbInfo.size = m_size;
    vbInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | usageFlags;
    vbInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo stagingAllocInfo = {};
    stagingAllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    stagingAllocInfo.flags = 0;
    ERR_GUARD_VULKAN(vmaCreateBuffer(m_device->getVMAAllocator(), &vbInfo, &stagingAllocInfo,
                                     &m_hBuffer,
                                     &m_hBufferAlloc, &allocationInfo));

    device->setObjectName((uint64_t) m_hBuffer, VK_OBJECT_TYPE_BUFFER, obj_name);

#ifdef DUMP_SELECTION_OF_MEMTYPE
    std::cout << "GPU Buff "<< obj_name <<", size = " << size << ", memtype = " << allocationInfo.memoryType << std::endl;
#endif

}

BufferGpuVLK::~BufferGpuVLK() {
    //This thing MUST ONLY ALLOCATE FUTURE DEALLOC
    auto l_device = m_device;
    auto l_buffer = m_hBuffer;
    auto l_bufferAlloc = m_hBufferAlloc;
    m_device->addDeallocationRecord(
        [l_buffer, l_device, l_bufferAlloc]() {
            vmaDestroyBuffer(l_device->getVMAAllocator(), l_buffer, l_bufferAlloc);
        }
    );
}
