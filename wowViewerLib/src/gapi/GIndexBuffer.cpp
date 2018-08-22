//
// Created by deamon on 05.06.18.
//
#include "../engine/opengl/header.h"
#include "GIndexBuffer.h"

GIndexBuffer::GIndexBuffer(GDevice &device) : m_device(device) {
    buffer = new GLuint;
    createBuffer();
}
GIndexBuffer::~GIndexBuffer(){
    destroyBuffer();
    delete (GLuint *)buffer;
}

void GIndexBuffer::createBuffer() {
    glGenBuffers(1, (GLuint *) this->buffer);
}

void GIndexBuffer::destroyBuffer() {
    glDeleteBuffers(1, (const GLuint *) this->buffer);
}

void GIndexBuffer::uploadData(void * data, int length) {
    m_device.bindVertexBufferBindings(nullptr);
    m_device.bindIndexBuffer(this);

    assert(m_buffCreated);

    if (!m_dataUploaded || length > m_size) {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, length, data, GL_DYNAMIC_DRAW);
        m_size = (size_t) length;
    } else {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, length, nullptr, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, length, data);
//        void * mapped = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, length, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
//        memcpy(mapped, data, length);
//        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    }

    m_device.bindIndexBuffer(nullptr);
    m_dataUploaded = true;
}

void GIndexBuffer::bind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *(GLuint*) this->buffer);
}

void GIndexBuffer::unbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
