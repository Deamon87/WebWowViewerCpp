//
// Created by deamon on 05.06.18.
//

#include <iostream>
#include "../GDeviceGL20.h"
#include "../../../engine/opengl/header.h"



GVertexBufferGL20::GVertexBufferGL20(IDevice &device)  : m_device(device) {
    pIdentifierBuffer = std::vector<char>(sizeof(GLuint));
    createBuffer();
}
GVertexBufferGL20::~GVertexBufferGL20() {
    destroyBuffer();
}

void GVertexBufferGL20::createBuffer() {
    glGenBuffers(1, (GLuint *)&this->pIdentifierBuffer[0]);
    m_buffCreated = true;
}

void GVertexBufferGL20::destroyBuffer() {
    glDeleteBuffers(1, (GLuint *)&this->pIdentifierBuffer[0]);
}

static int vbo_uploaded = 0;
void GVertexBufferGL20::uploadData(void * data, int length) {
    m_device.bindVertexBufferBindings(nullptr);
    m_device.bindVertexBuffer(this);

    assert(m_buffCreated);
    if (!(length > 0 && length < (400*1024*1024))) {
        //std::cout << "hello!";
    }
    assert(length > 0 && length < (400*1024*1024));

//    std::cout << "vbo_uploaded = " << vbo_uploaded++ << std::endl;

    if (!m_dataUploaded || length > m_size) {
        glBufferData(GL_ARRAY_BUFFER, length, data, GL_STATIC_DRAW);
        m_size = (size_t) length;
    } else {
        glBufferData(GL_ARRAY_BUFFER, length, nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, length, data);
//        void * mapped = glMapBufferRange(GL_ARRAY_BUFFER, 0, length, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
//        memcpy(mapped, data, length);
//        glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    m_device.bindVertexBuffer(nullptr);
    m_dataUploaded = true;
}

void GVertexBufferGL20::bind() {
    glBindBuffer(GL_ARRAY_BUFFER, *(GLuint *) &this->pIdentifierBuffer[0]);
}

void GVertexBufferGL20::unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
