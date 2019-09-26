//
// Created by deamon on 05.06.18.
//

#include <iostream>
#include <cstring>
#include "GVertexBufferVLK.h"

GVertexBufferVLK::GVertexBufferVLK(IDevice &device) : m_device(dynamic_cast<GDeviceVLK &>(device)) {

    createBuffer();
}

GVertexBufferVLK::~GVertexBufferVLK() {
    destroyBuffer();

}

void GVertexBufferVLK::createBuffer() {

}

void GVertexBufferVLK::destroyBuffer() {

}

static int vbo_uploaded = 0;

void GVertexBufferVLK::uploadData(void *data, int length) {
    if (!m_dataUploaded || length > m_size) {
        //TODO: free previous memory
        if (m_dataUploaded) {
            auto *l_device = &m_device;
            auto &l_stagingVertexBuffer = stagingVertexBuffer;
            auto &l_stagingVertexBufferAlloc = stagingVertexBufferAlloc;

            m_device.addDeallocationRecord(
                [l_device, l_stagingVertexBuffer, l_stagingVertexBufferAlloc]() {
                    vmaDestroyBuffer(l_device->getVMAAllocator(), l_stagingVertexBuffer, l_stagingVertexBufferAlloc);
                }
            );
        }

        //Create new buffer for VBO
        VkBufferCreateInfo vbInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        vbInfo.size = length;
        vbInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        vbInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo ibAllocCreateInfo = {};
        ibAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        ibAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        vmaCreateBuffer(m_device.getVMAAllocator(), &vbInfo, &ibAllocCreateInfo, &stagingVertexBuffer,
                                         &stagingVertexBufferAlloc, &stagingVertexBufferAllocInfo);

        memcpy(stagingVertexBufferAllocInfo.pMappedData, data, length);

        // No need to flush stagingVertexBuffer memory because CPU_ONLY memory is always HOST_COHERENT.

        vbInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        ibAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        ibAllocCreateInfo.flags = 0;
        ERR_GUARD_VULKAN(vmaCreateBuffer(m_device.getVMAAllocator(), &vbInfo, &ibAllocCreateInfo, &g_hVertexBuffer,
                                         &g_hVertexBufferAlloc, nullptr));


        VkBufferCopy vbCopyRegion = {};
        vbCopyRegion.srcOffset = 0;
        vbCopyRegion.dstOffset = 0;
        vbCopyRegion.size = vbInfo.size;
        vkCmdCopyBuffer(m_device.getUploadCommandBuffer(), stagingVertexBuffer, g_hVertexBuffer, 1, &vbCopyRegion);

        m_size = vbInfo.size;
        m_dataUploaded = true;
    } else {
        memcpy(stagingVertexBufferAllocInfo.pMappedData, data, length);

        VkBufferCopy vbCopyRegion = {};
        vbCopyRegion.srcOffset = 0;
        vbCopyRegion.dstOffset = 0;
        vbCopyRegion.size = length;
        vkCmdCopyBuffer(m_device.getUploadCommandBuffer(), stagingVertexBuffer, g_hVertexBuffer, 1, &vbCopyRegion);
    }


}

void GVertexBufferVLK::bind() {

}

void GVertexBufferVLK::unbind() {

}
