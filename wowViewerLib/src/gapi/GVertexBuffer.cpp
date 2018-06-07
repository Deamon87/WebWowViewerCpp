//
// Created by deamon on 05.06.18.
//
#include "../engine/opengl/header.h"
#include "GVertexBuffer.h"

GVertexBuffer::GVertexBuffer(GDevice &device)  : m_device(device) {
    buffer = new GLuint;
    createBuffer();
}
GVertexBuffer::~GIndexBuffer() {
    destroyBuffer();
    delete (GLuint *)buffer;
}

void GVertexBuffer::createBuffer() {
    glGenBuffers(1, (GLuint *)this->buffer);
}

void GVertexBuffer::destroyBuffer() {
    glDeleteBuffers(1, (GLuint *)this->buffer);
}

void GVertexBuffer::uploadData(void * data, int length) {
    m_device.bindVertexBuffer(this);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, length, data, GL_STATIC_DRAW);
}

void GVertexBuffer::bind() {
    glBindBuffer(GL_ARRAY_BUFFER, *(GLuint *) this->buffer);
}

void GVertexBuffer::unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
