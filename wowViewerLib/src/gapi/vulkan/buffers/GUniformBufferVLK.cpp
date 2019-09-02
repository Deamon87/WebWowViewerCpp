//
// Created by Deamon on 6/30/2018.
//
#include <memory.h>
#include <iostream>
#include "GUniformBufferVLK.h"
#include "../../interface/IDevice.h"

GUniformBufferVLK::GUniformBufferVLK(IDevice &device, size_t size) : m_device(device){
    m_size = size;
    pFrameOneContent = new char[size];
//    createBuffer();
}

GUniformBufferVLK::~GUniformBufferVLK() {
    if (m_buffCreated) {
        destroyBuffer();
    }

    delete (char *)pFrameOneContent;
}

void GUniformBufferVLK::createBuffer() {
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
    assert(length > 0 && length <= 65536);

    m_dataUploaded = true;
    m_needsUpdate = false;
}

void GUniformBufferVLK::save(bool initialSave) {
//    if (memcmp(pPreviousContent, pContent, m_size) != 0) {
        //1. Copy new to prev
        m_needsUpdate = true;

//        2. Update UBO
        if (m_buffCreated) {
            void * data = getPointerForModification();
            this->uploadData(data, m_size);
        }
//    }
}

void *GUniformBufferVLK::getPointerForUpload() {
    return pFrameOneContent;
}

void *GUniformBufferVLK::getPointerForModification() {
    return pFrameOneContent;

}

