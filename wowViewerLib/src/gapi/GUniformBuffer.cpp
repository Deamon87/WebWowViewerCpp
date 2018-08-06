//
// Created by Deamon on 6/30/2018.
//
#include <memory.h>
#include "../engine/opengl/header.h"
#include "GUniformBuffer.h"

GUniformBuffer::GUniformBuffer(GDevice &device, size_t size) : m_device(device){
    m_size = size;
    pIdentifierBuffer = new GLuint;
    pContent = new char[size];
    pPreviousContent = new char[size];
//    createBuffer();
}

GUniformBuffer::~GUniformBuffer() {
    if (m_buffCreated) {
        destroyBuffer();
    }
    delete (GLuint *)pIdentifierBuffer;
    delete (char *)pPreviousContent;
    delete (char *)pContent;
}

void GUniformBuffer::createBuffer() {
    glGenBuffers(1, (GLuint *)this->pIdentifierBuffer);
    m_buffCreated = true;

}


void GUniformBuffer::destroyBuffer() {
    glDeleteBuffers(1, (GLuint *)this->pIdentifierBuffer);
}
void GUniformBuffer::bind(int bindingPoint) { //Should be called only by GDevice
    if (m_buffCreated && bindingPoint == -1) {
        glBindBuffer(GL_UNIFORM_BUFFER, *(GLuint *) this->pIdentifierBuffer);
    } else if (m_buffCreated) {
        glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, *(GLuint *) this->pIdentifierBuffer);
    } else {
        glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, *(GLuint *) this->pIdentifierBuffer, m_offset, m_size);
    }
}
void GUniformBuffer::unbind() {
//    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GUniformBuffer::uploadData(void * data, int length) {
    m_device.bindVertexUniformBuffer(this, -1);

    if (!m_dataUploaded || length > m_size) {
        glBufferData(GL_UNIFORM_BUFFER, length, data, GL_DYNAMIC_DRAW);

        m_size = (size_t) length;
    } else {
//        glBufferData(GL_UNIFORM_BUFFER, length, data, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, length, data);
    }

    m_dataUploaded = true;
    m_needsUpdate = false;
}

void GUniformBuffer::save() {
//    if (memcmp(pPreviousContent, pContent, m_size) != 0) {
        //1. Copy new to prev
//        memcpy(pPreviousContent, pContent, m_size);
        m_needsUpdate = true;

//        2. Update UBO
        if (m_buffCreated) {
            this->uploadData(pContent, m_size);
        }
//    }
}
