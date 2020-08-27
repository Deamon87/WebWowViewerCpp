//
// Created by deamon on 05.06.18.
//

#include <iostream>
#include "../GDeviceGL33.h"
#include "GVertexBufferDynamicGL33.h"
#include "../../../engine/opengl/header.h"



GVertexBufferDynamicGL33::GVertexBufferDynamicGL33(IDevice &device, size_t maxSize)  : m_device(
    dynamic_cast<GDeviceGL33 &>(device)) {
    pIdentifierBuffer = std::vector<char>(sizeof(GLuint));
    m_size = maxSize;
	m_buffer.resize(maxSize);
    createBuffer();
}
GVertexBufferDynamicGL33::~GVertexBufferDynamicGL33() {
	destroyBuffer();
}

void GVertexBufferDynamicGL33::createBuffer() {
    glGenBuffers(1, (GLuint *)&this->pIdentifierBuffer[0]);
    m_buffCreated = true;
}

void GVertexBufferDynamicGL33::destroyBuffer() {
    const GLuint indent = *(const GLuint *) &this->pIdentifierBuffer[0];
    m_device.addDeallocationRecord([indent]() -> void {
        glDeleteBuffers(1, &indent);
    });
}

static int vbo_uploaded = 0;
void GVertexBufferDynamicGL33::uploadData(void * data, int length) {
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

void GVertexBufferDynamicGL33::bind() {
    glBindBuffer(GL_ARRAY_BUFFER, *(GLuint *) &this->pIdentifierBuffer[0]);
}

void GVertexBufferDynamicGL33::unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void *GVertexBufferDynamicGL33::getPointerForModification() {
    return &m_buffer[0];
}

void GVertexBufferDynamicGL33::save(size_t sizeToSave) {
    this->uploadData(&m_buffer[0], sizeToSave);
}
void GVertexBufferDynamicGL33::resize(size_t sizeToSave) {
    if (sizeToSave > m_size) {
//        destroyBuffer();
        m_size = sizeToSave;
        m_buffer.resize(sizeToSave);
//        createBuffer();


    }
}