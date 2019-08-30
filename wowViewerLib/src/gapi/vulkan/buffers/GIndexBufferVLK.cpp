//
// Created by deamon on 05.06.18.
//
#include <iostream>
#include <cstring>
#include "GIndexBufferVLK.h"

GIndexBufferVLK::GIndexBufferVLK(IDevice &device) : m_device(dynamic_cast<GDeviceVLK &>(device)) {
    createBuffer();
}
GIndexBufferVLK::~GIndexBufferVLK(){
    destroyBuffer();
}

void GIndexBufferVLK::createBuffer() {
}

void GIndexBufferVLK::destroyBuffer() {
}

static int ibo_uploaded = 0;
void GIndexBufferVLK::uploadData(void * data, int length) {
#define INTEL_MESA_PADDING 12

    if (!m_dataUploaded || length > m_size) {
        //TODO: create

        VkBufferCreateInfo ibInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        ibInfo.size = length;
        ibInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        ibInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo ibAllocCreateInfo = {};
        ibAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        ibAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VmaAllocationInfo stagingIndexBufferAllocInfo = {};
        ERR_GUARD_VULKAN(vmaCreateBuffer(m_device.getVMAAllocator(), &ibInfo, &ibAllocCreateInfo, &stagingIndexBuffer,
                                         &stagingIndexBufferAlloc, &stagingIndexBufferAllocInfo));

        memcpy(stagingIndexBufferAllocInfo.pMappedData, data, length);

        // No need to flush stagingIndexBuffer memory because CPU_ONLY memory is always HOST_COHERENT.
//        ibInfo.size += INTEL_MESA_PADDING; //Padding is required by Intel's drivers
        ibInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        ibAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        ibAllocCreateInfo.flags = 0;
        ERR_GUARD_VULKAN(vmaCreateBuffer(m_device.getVMAAllocator(), &ibInfo, &ibAllocCreateInfo, &g_hIndexBuffer,
                                         &g_hIndexBufferAlloc, nullptr));


        VkBufferCopy vbCopyRegion = {};
        vbCopyRegion.srcOffset = 0;
        vbCopyRegion.dstOffset = 0;
        vbCopyRegion.size = ibInfo.size;
        vkCmdCopyBuffer(m_device.getUploadCommandBuffer(), stagingIndexBuffer, g_hIndexBuffer, 1, &vbCopyRegion);

        m_dataUploaded = true;

        assert(length > 0 && length < (400 * 1024 * 1024));

//    std::cout << "vbo_uploaded = " << vbo_uploaded++ << std::endl;


    } else {
        //TODO:!!!!
    }

}

void GIndexBufferVLK::bind() {
}

void GIndexBufferVLK::unbind() {
}
