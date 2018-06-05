//
// Created by deamon on 05.06.18.
//

#include "GIndexBuffer.h"

void GIndexBuffer::createBuffer() {
    glGenBuffers(1, &this->buffer);
}

void GIndexBuffer::destroyBuffer() {
    glDeleteBuffers(1, &this->buffer);
}

void GIndexBuffer::uploadData(void * data, int length) {
    m_device.bindIndexBuffer(this);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, length, data, GL_STATIC_DRAW);
}
