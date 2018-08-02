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
    m_device.bindIndexBuffer(this);

    if (!m_dataUploaded || length > m_size) {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, length, data, GL_DYNAMIC_DRAW);
        m_size = (size_t) length;
    } else {
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, length, data);
    }

    m_dataUploaded = true;
}

void GIndexBuffer::bind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *(GLuint*) this->buffer);
}

void GIndexBuffer::unbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
