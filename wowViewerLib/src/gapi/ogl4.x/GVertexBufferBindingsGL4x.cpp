//
// Created by deamon on 05.06.18.
//

#include "../../engine/opengl/header.h"
#include "GVertexBufferBindingsGL4x.h"
#include "../interface/IDevice.h"

constexpr GLenum toOGLEnum(GBindingType bindingType) {
    switch (bindingType) {
        case GBindingType::GFLOAT :
            return GL_FLOAT;
        case GBindingType::GUNSIGNED_BYTE:
            return GL_UNSIGNED_BYTE;
        default:
            throw std::runtime_error("unknown GBindingType");
    }
}

GVertexBufferBindingsGL4x::GVertexBufferBindingsGL4x(IDevice &m_device) : m_device(m_device) {
    m_buffer = new GLuint;
    createBuffer();
}

GVertexBufferBindingsGL4x::~GVertexBufferBindingsGL4x() {
    destroyBuffer();
}

void GVertexBufferBindingsGL4x::createBuffer() {
    glGenVertexArrays(1, (GLuint *)this->m_buffer);
}

void GVertexBufferBindingsGL4x::destroyBuffer() {
    glDeleteVertexArrays(1, (GLuint *)this->m_buffer);
}

void GVertexBufferBindingsGL4x::bind() {
    glBindVertexArray(*(GLuint *)this->m_buffer);
}

void GVertexBufferBindingsGL4x::unbind() {
    glBindVertexArray(0);
}

void GVertexBufferBindingsGL4x::setIndexBuffer(HGIndexBuffer indexBuffer) {
    m_indexBuffer = indexBuffer;
}

void GVertexBufferBindingsGL4x::addVertexBufferBinding(GVertexBufferBinding binding) {
    m_bindings.push_back(binding);
}

void GVertexBufferBindingsGL4x::save() {
    m_device.bindVertexBufferBindings(this);
//    for (GVertexBufferBinding &binding : m_bindings) {
//        for (GBufferBinding &bufferBinding : binding.bindings) {
//
//        }
//    }
//    m_device.bindIndexBuffer(nullptr);
//    m_device.bindVertexBuffer(nullptr);
    for (GVertexBufferBinding &binding : m_bindings) {
        m_device.bindVertexBuffer(binding.vertexBuffer.get());

        for (GBufferBinding &bufferBinding : binding.bindings) {
            glEnableVertexAttribArray(bufferBinding.position);
            glVertexAttribPointer(
                bufferBinding.position,
                bufferBinding.size,
                toOGLEnum(bufferBinding.type),
                (GLboolean) (bufferBinding.normalized ? GL_TRUE : GL_FALSE),
                bufferBinding.stride,
                (const void *) bufferBinding.offset
            );
        }
    }

    m_device.bindIndexBuffer(m_indexBuffer.get());
    m_device.bindVertexBufferBindings(nullptr);

}



