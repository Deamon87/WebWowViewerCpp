//
// Created by Deamon on 6/30/2018.
//
#include <memory.h>
#include <iostream>
#include "../../../engine/opengl/header.h"
#include "GUniformBufferGL33.h"
#include "../../interface/IDevice.h"

GUniformBufferGL33::GUniformBufferGL33(IDevice &device, size_t size) : m_device(dynamic_cast<GDeviceGL33 &>(device)){
    m_size = size;
    createBuffer();
}

GUniformBufferGL33::~GUniformBufferGL33() {
    if (m_buffCreated) {
        destroyBuffer();
    }
}

void GUniformBufferGL33::createBuffer() {
    glGenBuffers(1, (GLuint *) &glBuffId);
    m_buffCreated = true;
}

void GUniformBufferGL33::destroyBuffer() {
    const GLuint indent = glBuffId;
    m_device.addDeallocationRecord([indent]() -> void {
        glDeleteBuffers(1, &indent);
    });
}
void GUniformBufferGL33::bind(int bindingPoint, int offset, int length) { //Should be called only by GDevice
    if (bindingPoint == 0 && offset == 0 && length == 0) {
        glBindBuffer(GL_UNIFORM_BUFFER, glBuffId);
    } else {
        glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, glBuffId, offset, length);
    }
}
void GUniformBufferGL33::unbind() {
//    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GUniformBufferGL33::uploadData(void * data, int length) {
    m_device.bindUniformBuffer(this, 0, 0, 0);

    assert(m_buffCreated);
#ifdef __EMSCRIPTEN__
    assert(length > 0 && length <= 65536);
#endif

    if (!m_dataUploaded || m_size < length) {
        glBufferData(GL_UNIFORM_BUFFER, length, data, GL_DYNAMIC_DRAW);
        m_size = (size_t) length;
    } else {
        glBufferSubData(GL_UNIFORM_BUFFER, 0, length, data);
    }

    m_dataUploaded = true;
}
