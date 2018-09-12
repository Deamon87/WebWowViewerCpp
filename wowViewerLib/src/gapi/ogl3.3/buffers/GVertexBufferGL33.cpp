//
// Created by deamon on 05.06.18.
//

#include "../GDeviceGL33.h"
#include "../../../engine/opengl/header.h"



GVertexBufferGL33::GVertexBufferGL33(IDevice &device)  : m_device(device) {
    pIdentifierBuffer = new GLuint;
    createBuffer();
}
GVertexBufferGL33::~GVertexBufferGL33() {
    destroyBuffer();
    delete (GLuint *)pIdentifierBuffer;
}

void GVertexBufferGL33::createBuffer() {
    glGenBuffers(1, (GLuint *)this->pIdentifierBuffer);
    m_buffCreated = true;
}

void GVertexBufferGL33::destroyBuffer() {
    glDeleteBuffers(1, (GLuint *)this->pIdentifierBuffer);
}

void GVertexBufferGL33::uploadData(void * data, int length) {
    m_device.bindVertexBufferBindings(nullptr);
    m_device.bindVertexBuffer(this);

    if (length <= 0) return;
    if (data == nullptr) return;

    assert(m_buffCreated);

    if (!m_dataUploaded || length > m_size) {
        glBufferData(GL_ARRAY_BUFFER, length, data, GL_DYNAMIC_DRAW);
        m_size = (size_t) length;
    } else {
        glBufferData(GL_ARRAY_BUFFER, length, nullptr, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, length, data);
//        void * mapped = glMapBufferRange(GL_ARRAY_BUFFER, 0, length, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
//        memcpy(mapped, data, length);
//        glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    m_device.bindVertexBuffer(nullptr);
    m_dataUploaded = true;
}

void GVertexBufferGL33::bind() {
    glBindBuffer(GL_ARRAY_BUFFER, *(GLuint *) this->pIdentifierBuffer);
}

void GVertexBufferGL33::unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
