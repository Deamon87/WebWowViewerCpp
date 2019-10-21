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
    if (m_dataUploaded) {
        auto *l_device = &m_device;

        auto &l_stagingIndexBuffer = stagingIndexBuffer;
        auto &l_stagingIndexBufferAlloc = stagingIndexBufferAlloc;

        auto &l_hIndexBuffer = g_hIndexBuffer;
        auto &l_hIndexBufferAlloc = g_hIndexBufferAlloc;

        m_device.addDeallocationRecord(
            [l_device, l_stagingIndexBuffer, l_stagingIndexBufferAlloc, l_hIndexBuffer, l_hIndexBufferAlloc]() {
                vmaDestroyBuffer(l_device->getVMAAllocator(), l_stagingIndexBuffer, l_stagingIndexBufferAlloc);
                vmaDestroyBuffer(l_device->getVMAAllocator(), l_hIndexBuffer, l_hIndexBufferAlloc);
            }
        );
    }
}

static int ibo_uploaded = 0;
void GIndexBufferVLK::uploadData(void * data, int length) {
    if (!m_dataUploaded || length > m_size) {
        if (m_dataUploaded) {
            auto *l_device = &m_device;
            auto &l_stagingIndexBuffer = stagingIndexBuffer;
            auto &l_stagingIndexBufferAlloc = stagingIndexBufferAlloc;

            auto &l_hIndexBuffer = g_hIndexBuffer;
            auto &l_hIndexBufferAlloc = g_hIndexBufferAlloc;

            m_device.addDeallocationRecord(
                [l_device, l_stagingIndexBuffer, l_stagingIndexBufferAlloc, l_hIndexBuffer, l_hIndexBufferAlloc]() {
                    vmaDestroyBuffer(l_device->getVMAAllocator(), l_stagingIndexBuffer, l_stagingIndexBufferAlloc);
                    vmaDestroyBuffer(l_device->getVMAAllocator(), l_hIndexBuffer, l_hIndexBufferAlloc);
                }
            );
        }

        VkBufferCreateInfo ibInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        ibInfo.size = length;
        ibInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        ibInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo ibAllocCreateInfo = {};
        ibAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        ibAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;


        ERR_GUARD_VULKAN(vmaCreateBuffer(m_device.getVMAAllocator(), &ibInfo, &ibAllocCreateInfo, &stagingIndexBuffer,
                                         &stagingIndexBufferAlloc, &stagingIndexBufferAllocInfo));

        memcpy(stagingIndexBufferAllocInfo.pMappedData, data, length);

        // No need to flush stagingIndexBuffer memory because CPU_ONLY memory is always HOST_COHERENT.
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

        m_size = ibInfo.size;
        m_dataUploaded = true;
//    std::cout << "vbo_uploaded = " << vbo_uploaded++ << std::endl;
    } else {
        memcpy(stagingIndexBufferAllocInfo.pMappedData, data, length);

        VkBufferCopy vbCopyRegion = {};
        vbCopyRegion.srcOffset = 0;
        vbCopyRegion.dstOffset = 0;
        vbCopyRegion.size = length;
        vkCmdCopyBuffer(m_device.getUploadCommandBuffer(), stagingIndexBuffer, g_hIndexBuffer, 1, &vbCopyRegion);
    }
}

void GIndexBufferVLK::bind() {
}

void GIndexBufferVLK::unbind() {
}
