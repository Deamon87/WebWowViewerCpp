//
// Created by deamon on 05.06.18.
//
#include <iostream>
#include "../../../engine/opengl/header.h"
#include "GIndexBufferGL20.h"
#include "../GDeviceGL20.h"

GIndexBufferGL20::GIndexBufferGL20(IDevice &device) : m_device(device) {
    buffer = std::vector<char>(sizeof(GLuint));
    createBuffer();
}
GIndexBufferGL20::~GIndexBufferGL20(){
    destroyBuffer();
}

void GIndexBufferGL20::createBuffer() {
    glGenBuffers(1, (GLuint *) &this->buffer[0]);
}

void GIndexBufferGL20::destroyBuffer() {
    glDeleteBuffers(1, (const GLuint *) &this->buffer[0]);
}

static int ibo_uploaded = 0;
void GIndexBufferGL20::uploadData(void * data, int length) {
    m_device.bindVertexBufferBindings(nullptr);
    m_device.bindIndexBuffer(this);

    if (length <= 0) return;
    if (data == nullptr) return;

    assert(m_buffCreated);
    assert(length > 0 && length < (400*1024*1024));

//    std::cout << "ibo_uploaded = " << ibo_uploaded++ << std::endl;

    if (!m_dataUploaded || length > m_size) {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, length, data, GL_STATIC_DRAW);
        m_size = (size_t) length;
    } else {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, length, nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, length, data);
//        void * mapped = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, length, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
//        memcpy(mapped, data, length);
//        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    }

    m_device.bindIndexBuffer(nullptr);
    m_dataUploaded = true;
}

void GIndexBufferGL20::bind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *(GLuint*) &this->buffer[0]);
}

void GIndexBufferGL20::unbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
