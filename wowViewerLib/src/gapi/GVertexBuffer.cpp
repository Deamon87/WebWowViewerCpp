//
// Created by deamon on 05.06.18.
//
#include "../engine/opengl/header.h"
#include "GVertexBuffer.h"

GVertexBuffer::GVertexBuffer(GDevice &device)  : m_device(device) {
    pIdentifierBuffer = new GLuint;
    createBuffer();
}
GVertexBuffer::~GVertexBuffer() {
    destroyBuffer();
    delete (GLuint *)pIdentifierBuffer;
}

void GVertexBuffer::createBuffer() {
    glGenBuffers(1, (GLuint *)this->pIdentifierBuffer);
}

void GVertexBuffer::destroyBuffer() {
    glDeleteBuffers(1, (GLuint *)this->pIdentifierBuffer);
}

void GVertexBuffer::uploadData(void * data, int length) {
    m_device.bindVertexBuffer(this);

    if (!m_dataUploaded || length > m_size) {
        glBufferData(GL_ARRAY_BUFFER, length, data, GL_STATIC_DRAW);
        m_size = (size_t) length;
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, length, data);
    }

    m_dataUploaded = true;
}

void GVertexBuffer::bind() {
    glBindBuffer(GL_ARRAY_BUFFER, *(GLuint *) this->pIdentifierBuffer);
}

void GVertexBuffer::unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
