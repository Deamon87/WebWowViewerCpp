//
// Created by deamon on 05.06.18.
//

#include <iostream>
#include <cstring>
#include "GVertexBufferDynamicVLK.h"

GVertexBufferDynamicVLK::GVertexBufferDynamicVLK(IDevice &device, size_t maxSize) : m_device(dynamic_cast<GDeviceVLK &>(device)) {
    m_size = maxSize;
    createBuffer();
}

GVertexBufferDynamicVLK::~GVertexBufferDynamicVLK() {
    destroyBuffer();

}

void GVertexBufferDynamicVLK::createBuffer() {
    //Create new buffer for VBO
    VkBufferCreateInfo vbInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    vbInfo.size = m_size;
    vbInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vbInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo ibAllocCreateInfo = {};
    ibAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    ibAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    ERR_GUARD_VULKAN(vmaCreateBuffer(m_device.getVMAAllocator(), &vbInfo, &ibAllocCreateInfo, &stagingVertexBuffer,
                                     &stagingVertexBufferAlloc, &stagingVertexBufferAllocInfo));



    vbInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    ibAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    ibAllocCreateInfo.flags = 0;
    ERR_GUARD_VULKAN(vmaCreateBuffer(m_device.getVMAAllocator(), &vbInfo, &ibAllocCreateInfo, &g_hVertexBuffer,
                                     &g_hVertexBufferAlloc, nullptr));
}

void GVertexBufferDynamicVLK::destroyBuffer() {
    auto *l_device = &m_device;
    auto &l_stagingVertexBuffer = stagingVertexBuffer;
    auto &l_stagingVertexBufferAlloc = stagingVertexBufferAlloc;

    auto &l_hVertexBuffer = g_hVertexBuffer;
    auto &l_hVertexBufferAlloc = g_hVertexBufferAlloc;

    m_device.addDeallocationRecord(
        [l_device, l_stagingVertexBuffer, l_stagingVertexBufferAlloc, l_hVertexBuffer, l_hVertexBufferAlloc]() {
            vmaDestroyBuffer(l_device->getVMAAllocator(), l_stagingVertexBuffer, l_stagingVertexBufferAlloc);
            vmaDestroyBuffer(l_device->getVMAAllocator(), l_hVertexBuffer, l_hVertexBufferAlloc);
        }
    );
}

static int vbo_uploaded = 0;

void GVertexBufferDynamicVLK::uploadData(void *data, int length) {
}

void GVertexBufferDynamicVLK::bind() {

}

void GVertexBufferDynamicVLK::unbind() {

}

void *GVertexBufferDynamicVLK::getPointerForModification() {
    return stagingVertexBufferAllocInfo.pMappedData;
}

void GVertexBufferDynamicVLK::save(size_t sizeToSave) {
    VkBufferCopy vbCopyRegion = {};
    vbCopyRegion.srcOffset = 0;
    vbCopyRegion.dstOffset = 0;
    vbCopyRegion.size = sizeToSave;
    vkCmdCopyBuffer(m_device.getUploadCommandBuffer(), stagingVertexBuffer, g_hVertexBuffer, 1, &vbCopyRegion);
}

void GVertexBufferDynamicVLK::resize(size_t sizeToSave) {
    if (sizeToSave > m_size) {
        destroyBuffer();
        m_size = sizeToSave;
        createBuffer();
    }
}