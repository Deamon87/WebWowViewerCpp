//
// Created by deamon on 05.06.18.
//

#include "GVertexBuffer.h"

void GVertexBuffer::createBuffer() {
    glGenBuffers(1, &this->buffer);
}

void GVertexBuffer::destroyBuffer() {
    glDeleteBuffers(1, &this->buffer);
}

void GVertexBuffer::uploadData(void * data, int length) {
    m_device.bindVertexBuffer(this);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, length, data, GL_STATIC_DRAW);
}
