//
// Created by Deamon on 6/30/2018.
//
#include <memory.h>
#include "../../../engine/opengl/header.h"
#include "GUniformBufferGL33.h"
#include "../../interface/IDevice.h"

GUniformBufferGL33::GUniformBufferGL33(IDevice &device, size_t size) : m_device(device){
    m_size = size;
    pIdentifierBuffer[0] = new GLuint;
    pIdentifierBuffer[1] = nullptr;
    pFrameOneContent = new char[size];
    pFrameTwoContent = new char[size];
//    createBuffer();
}

GUniformBufferGL33::~GUniformBufferGL33() {
    if (m_buffCreated) {
        destroyBuffer();
    }
    delete (GLuint *)pIdentifierBuffer[0];
    delete (char *)pFrameOneContent;
    delete (char *)pFrameTwoContent;
}

void GUniformBufferGL33::createBuffer() {
    glGenBuffers(1, (GLuint *)this->pIdentifierBuffer[0]);
    m_buffCreated = true;

}


void GUniformBufferGL33::destroyBuffer() {
    glDeleteBuffers(1, (GLuint *)this->pIdentifierBuffer[0]);
}
void GUniformBufferGL33::bind(int bindingPoint) { //Should be called only by GDevice
    if (m_buffCreated && bindingPoint == -1) {
        glBindBuffer(GL_UNIFORM_BUFFER, *(GLuint *) this->pIdentifierBuffer[0]);
    } else if (m_buffCreated) {
        glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, *(GLuint *) this->pIdentifierBuffer[0]);
    } else {
        glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, *(GLuint *) getIdentifierBuffer(), m_offset[m_device.getFrameNumber() & 1], m_size);
    }
}
void GUniformBufferGL33::unbind() {
//    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GUniformBufferGL33::uploadData(void * data, int length) {
    m_device.bindVertexUniformBuffer(this, -1);

    assert(m_buffCreated);

    glBufferData(GL_UNIFORM_BUFFER, length, nullptr, GL_STREAM_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, length, data);

    m_size = (size_t) length;

    m_dataUploaded = true;
    m_needsUpdate = false;
}

void GUniformBufferGL33::save(bool initialSave) {
//    if (memcmp(pPreviousContent, pContent, m_size) != 0) {
        //1. Copy new to prev
        m_needsUpdate = true;

//        2. Update UBO
        void * data = getPointerForModification();
        if (m_buffCreated) {
            this->uploadData(data, m_size);
        }
//    }
}

void *GUniformBufferGL33::getPointerForUpload() {
//    if (!m_device.getIsEvenFrame()) {
//        return pFrameTwoContent;
//    } else {
        return pFrameOneContent;
//    }
}

void *GUniformBufferGL33::getPointerForModification() {
//    if (m_device.getIsEvenFrame()) {
//        return pFrameTwoContent;
//    } else {
        return pFrameOneContent;
//    }
}

