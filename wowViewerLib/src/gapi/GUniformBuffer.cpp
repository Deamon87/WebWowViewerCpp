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
    createBuffer();
}

GUniformBuffer::~GUniformBuffer() {
    destroyBuffer();
    delete (GLuint *)pIdentifierBuffer;
    delete (char *)pPreviousContent;
    delete (char *)pContent;
}

void GUniformBuffer::createBuffer() {
    glGenBuffers(1, (GLuint *)this->pIdentifierBuffer);
}

void GUniformBuffer::destroyBuffer() {
    glDeleteBuffers(1, (GLuint *)this->pIdentifierBuffer);
}
void GUniformBuffer::bind() { //Should be called only by GDevice
    glBindBuffer(GL_UNIFORM_BUFFER, *(GLuint *) this->pIdentifierBuffer);
}
void GUniformBuffer::unbind() {
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GUniformBuffer::uploadData(void * data, int length) {
    m_device.bindUniformBuffer(this, 0);

    glBufferData(GL_UNIFORM_BUFFER, length, data, GL_STATIC_DRAW);

    m_needsUpdate = false;
}

void GUniformBuffer::save() {
    if (memcmp(pPreviousContent, pContent, m_size) != 0) {
        //1. Copy new to prev
        memcpy(pPreviousContent, pContent, m_size);
        m_needsUpdate = true;

//        2. Update UBO
//        this->uploadData(pPreviousContent, 0);
    }
}
