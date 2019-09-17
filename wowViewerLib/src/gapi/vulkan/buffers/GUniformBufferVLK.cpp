//
// Created by Deamon on 6/30/2018.
//
#include <memory.h>
#include <iostream>
#include "GUniformBufferVLK.h"
#include "../../interface/IDevice.h"

GUniformBufferVLK::GUniformBufferVLK(IDevice &device, size_t size) : m_device((GDeviceVLK *) &device){
    m_size = size;
    pFrameOneContent = new char[size];
    createBuffer();
}

GUniformBufferVLK::~GUniformBufferVLK() {
    if (m_buffCreated) {
        destroyBuffer();
    }

    delete (char *)pFrameOneContent;
}

void GUniformBufferVLK::createBuffer() {
    if (m_buffCreated)
        return;

    // Allocate a buffer out of it.
    VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufCreateInfo.size = m_size;
    bufCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.pool = m_device->getUBOPool();

    for (int i = 0; i < 4; i++) {
        vmaCreateBuffer(m_device->getVMAAllocator(), &bufCreateInfo, &allocCreateInfo, &g_buf[i], &g_alloc[i], &g_allocInfo[i]);
    }

    // CPU Buffer
    VkBufferCreateInfo uboInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    uboInfo.size = m_size;
    uboInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    uboInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo uboAllocCreateInfo = {};
    uboAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    uboAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;


    ERR_GUARD_VULKAN(vmaCreateBuffer(m_device->getVMAAllocator(), &uboInfo, &uboAllocCreateInfo, &stagingUBOBuffer,
                                     &stagingUBOBufferAlloc, &stagingUBOBufferAllocInfo ));

    m_buffCreated = true;
}

void GUniformBufferVLK::destroyBuffer() {
}
void GUniformBufferVLK::bind(int bindingPoint) { //Should be called only by GDevice

}
void GUniformBufferVLK::unbind() {
}

void GUniformBufferVLK::uploadData(void * data, int length) {
    assert(m_buffCreated);
    assert(length > 0 && length <= m_size);

    int updateIndex = m_device->getUpdateFrameNumber();

    memcpy(stagingUBOBufferAllocInfo.pMappedData, data, length);

    VkBufferCopy vbCopyRegion = {};
    vbCopyRegion.srcOffset = 0;
    vbCopyRegion.dstOffset = 0;
    vbCopyRegion.size = length;
    vkCmdCopyBuffer(m_device->getUploadCommandBuffer(), stagingUBOBuffer, g_buf[updateIndex], 1, &vbCopyRegion);

    m_dataUploaded = true;
    m_needsUpdate[updateIndex] = false;
}

void GUniformBufferVLK::save(bool initialSave) {
////    if (memcmp(pPreviousContent, pContent, m_size) != 0) {
//        //1. Copy new to prev
        m_needsUpdate[0] = true;
        m_needsUpdate[1] = true;
        m_needsUpdate[2] = true;
        m_needsUpdate[3] = true;
//
//        2. Update UBO

//    }
}

void *GUniformBufferVLK::getPointerForUpload() {
    return pFrameOneContent;
}

void *GUniformBufferVLK::getPointerForModification() {
    return pFrameOneContent;

}

void GUniformBufferVLK::commitUpload() {
    int updateIndex = m_device->getUpdateFrameNumber();
    if (m_buffCreated && m_needsUpdate[updateIndex]) {
        void * data = getPointerForModification();
        this->uploadData(data, m_size);
    }
}

