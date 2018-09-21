//
// Created by deamon on 05.06.18.
//

#include "../GDeviceGL4x.h"
#include "../../../engine/opengl/header.h"



GVertexBufferGL4x::GVertexBufferGL4x(IDevice &device)  : m_device(device) {
    pIdentifierBuffer = new GLuint;
    createBuffer();
}
GVertexBufferGL4x::~GVertexBufferGL4x() {
    destroyBuffer();
    delete (GLuint *)pIdentifierBuffer;
}

void GVertexBufferGL4x::createBuffer() {
    glGenBuffers(1, (GLuint *)this->pIdentifierBuffer);
    m_buffCreated = true;
}

void GVertexBufferGL4x::destroyBuffer() {
    glDeleteBuffers(1, (GLuint *)this->pIdentifierBuffer);
}

void GVertexBufferGL4x::uploadData(void * data, int length) {
    if (length <= 0) return;
    if (data == nullptr) return;

    assert(m_buffCreated);

    if (!m_dataUploaded) {
        m_device.bindVertexBufferBindings(nullptr);
        m_device.bindVertexBuffer(this);

//        glBufferStorage(GL_ARRAY_BUFFER, length, data, 0);
        glBufferData(GL_ARRAY_BUFFER, length, data, GL_DYNAMIC_DRAW);
        m_size = (size_t) length;
    } else {
//        if (m_isImmutable) {
//            destroyBuffer();
//            createBuffer();
//            m_isImmutable = false;
//        }
        m_device.bindVertexBufferBindings(nullptr);
        m_device.bindVertexBuffer(this);
        glBufferData(GL_ARRAY_BUFFER, length, nullptr, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, length, data);
//        void * mapped = glMapBufferRange(GL_ARRAY_BUFFER, 0, length, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
//        memcpy(mapped, data, length);
//        glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    m_device.bindVertexBuffer(nullptr);
    m_dataUploaded = true;
}

void GVertexBufferGL4x::bind() {
    glBindBuffer(GL_ARRAY_BUFFER, *(GLuint *) this->pIdentifierBuffer);
}

void GVertexBufferGL4x::unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
