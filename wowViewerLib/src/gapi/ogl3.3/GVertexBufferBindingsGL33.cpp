//
// Created by deamon on 05.06.18.
//

#include <iostream>
#include "../../engine/opengl/header.h"
#include "GVertexBufferBindingsGL33.h"
#include "../interface/IDevice.h"

GVertexBufferBindingsGL33::GVertexBufferBindingsGL33(IDevice &m_device) : m_device(m_device) {
    m_buffer = new GLuint;
    createBuffer();
}

GVertexBufferBindingsGL33::~GVertexBufferBindingsGL33() {
    destroyBuffer();
}

void GVertexBufferBindingsGL33::createBuffer() {
    glGenVertexArrays(1, (GLuint *)this->m_buffer);
}

void GVertexBufferBindingsGL33::destroyBuffer() {
    glDeleteVertexArrays(1, (GLuint *)this->m_buffer);
}

void GVertexBufferBindingsGL33::bind() {
    glBindVertexArray(*(GLuint *)this->m_buffer);
}

void GVertexBufferBindingsGL33::unbind() {
    glBindVertexArray(0);
}

void GVertexBufferBindingsGL33::setIndexBuffer(HGIndexBuffer indexBuffer) {
    m_indexBuffer = indexBuffer;
}

void GVertexBufferBindingsGL33::addVertexBufferBinding(GVertexBufferBinding binding) {
    m_bindings.push_back(binding);
}

static int VAO_updated = 0;
void GVertexBufferBindingsGL33::save() {
//    std::cout << "VAO_updated = " << VAO_updated++ << std::endl;

    m_device.bindVertexBufferBindings(this);
//    for (GVertexBufferBinding &binding : m_bindings) {
//        for (GBufferBinding &bufferBinding : binding.bindings) {
//
//        }
//    }
//    m_device.bindIndexBuffer(nullptr);
//    m_device.bindVertexBuffer(nullptr);
    m_device.bindIndexBuffer(m_indexBuffer.get());
    for (GVertexBufferBinding &binding : m_bindings) {
        m_device.bindVertexBuffer(binding.vertexBuffer.get());

        for (GBufferBinding &bufferBinding : binding.bindings) {
            glEnableVertexAttribArray(bufferBinding.position);
            glVertexAttribPointer(
                bufferBinding.position,
                bufferBinding.size,
                bufferBinding.type,
                (GLboolean) (bufferBinding.normalized ? GL_TRUE : GL_FALSE),
                bufferBinding.stride,
                (const void *) bufferBinding.offset
            );
        }
    }


    m_device.bindVertexBufferBindings(nullptr);

}



