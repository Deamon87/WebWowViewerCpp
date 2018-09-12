//
// Created by deamon on 05.06.18.
//
#include "../../../engine/opengl/header.h"
#include "GIndexBufferGL33.h"
#include "../GDeviceGL33.h"

GIndexBufferGL33::GIndexBufferGL33(IDevice &device) : m_device(device) {
    buffer = new GLuint;
    createBuffer();
}
GIndexBufferGL33::~GIndexBufferGL33(){
    destroyBuffer();
    delete (GLuint *)buffer;
}

void GIndexBufferGL33::createBuffer() {
    glGenBuffers(1, (GLuint *) this->buffer);
}

void GIndexBufferGL33::destroyBuffer() {
    glDeleteBuffers(1, (const GLuint *) this->buffer);
}

void GIndexBufferGL33::uploadData(void * data, int length) {
    m_device.bindVertexBufferBindings(nullptr);
    m_device.bindIndexBuffer(this);

    if (length <= 0) return;
    if (data == nullptr) return;

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

void GIndexBufferGL33::bind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *(GLuint*) this->buffer);
}

void GIndexBufferGL33::unbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
