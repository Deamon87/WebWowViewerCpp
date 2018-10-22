//
// Created by Deamon on 6/30/2018.
//
#include <memory.h>
#include "../../../engine/opengl/header.h"
#include "GUniformBufferGL4x.h"
#include "../../interface/IDevice.h"

GUniformBufferGL4x::GUniformBufferGL4x(IDevice &device, size_t size) : m_device(device){
    m_size = size;
    pIdentifierBuffer[0] = new GLuint;
    pIdentifierBuffer[1] = nullptr;
    pFrameOneContent = new char[size];
    pFrameTwoContent = new char[size];
//    createBuffer();
}

GUniformBufferGL4x::~GUniformBufferGL4x() {
    if (m_buffCreated) {
        destroyBuffer();
    }
    delete (GLuint *)pIdentifierBuffer[0];
    delete (char *)pFrameOneContent;
    delete (char *)pFrameTwoContent;
}

void GUniformBufferGL4x::createBuffer() {
    GLbitfield flags = GL_MAP_WRITE_BIT           |
                       GL_MAP_PERSISTENT_BIT |
                       GL_MAP_COHERENT_BIT;

    GLbitfield createFlags = flags | GL_DYNAMIC_STORAGE_BIT;
    glGenBuffers(1, (GLuint *)this->pIdentifierBuffer[0]);
    m_buffCreated = true;

    m_device.bindVertexUniformBuffer(this, -1);
    glBufferStorage( GL_UNIFORM_BUFFER, m_size, 0, createFlags );

    m_perisistentPointer = glMapBufferRange(
        GL_UNIFORM_BUFFER,
        0,
        m_size,
        flags );
}


void GUniformBufferGL4x::destroyBuffer() {
    glDeleteBuffers(1, (GLuint *)this->pIdentifierBuffer[0]);
}
void GUniformBufferGL4x::bind(int bindingPoint) { //Should be called only by GDevice
    if (m_buffCreated && bindingPoint == -1) {
        glBindBuffer(GL_UNIFORM_BUFFER, *(GLuint *) this->pIdentifierBuffer[0]);
    } else if (m_buffCreated) {
        glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, *(GLuint *) this->pIdentifierBuffer[0]);
    } else {
        glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, *(GLuint *) getIdentifierBuffer(), m_offset[m_device.getDrawFrameNumber()], m_size);
    }
}
void GUniformBufferGL4x::unbind() {
//    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GUniformBufferGL4x::uploadData(void * data, int length) {
    memcpy(m_perisistentPointer, data, length);

    m_dataUploaded = true;
    m_needsUpdate = false;
}

void GUniformBufferGL4x::save(bool initialSave) {
//    if (memcmp(pPreviousContent, pContent, m_size) != 0) {
        //1. Copy new to prev
        if (initialSave) {
            memcpy(getPointerForUpload(), getPointerForModification(), m_size);
        }
        m_needsUpdate = true;

//        2. Update UBO
        void * data = getPointerForModification();
        if (m_buffCreated) {
            this->uploadData(data, m_size);
        }
//    }
}

void *GUniformBufferGL4x::getPointerForUpload() {
//    if (!m_device.getIsEvenFrame()) {
//        return pFrameTwoContent;
//    } else {
        return pFrameOneContent;
//    }
}

void *GUniformBufferGL4x::getPointerForModification() {
//    if (m_device.getIsEvenFrame()) {
//        return pFrameTwoContent;
//    } else {
        return pFrameOneContent;
//    }
}

