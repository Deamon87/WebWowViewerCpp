//
// Created by deamon on 05.06.18.
//

#include <iostream>
#include "../../engine/opengl/header.h"
#include "GVertexBufferBindingsGL20.h"
#include "../interface/IDevice.h"

constexpr GLenum toOGLEnum(GBindingType bindingType) {
    switch (bindingType) {
        case GBindingType::GFLOAT :
            return GL_FLOAT;
        case GBindingType::GUNSIGNED_BYTE:
            return GL_UNSIGNED_BYTE;
//        default:
//            throw std::runtime_error("unknown GBindingType");
    }
    return 0;
}

GVertexBufferBindingsGL20::GVertexBufferBindingsGL20(IDevice &m_device) : m_device(m_device) {
    m_buffer = std::vector<char>(sizeof(GLuint));
//    createBuffer();
}

GVertexBufferBindingsGL20::~GVertexBufferBindingsGL20() {
//    destroyBuffer();
}

void GVertexBufferBindingsGL20::createBuffer() {
//    glGenVertexArrays(1, (GLuint *)&this->m_buffer[0]);
}

void GVertexBufferBindingsGL20::destroyBuffer() {
//    glDeleteVertexArrays(1, (GLuint *)&this->m_buffer[0]);
}

void GVertexBufferBindingsGL20::bind() {
//    glBindVertexArray(*(GLuint *)&this->m_buffer[0]);

    m_device.bindIndexBuffer(m_indexBuffer.get());
    for (GVertexBufferBinding &binding : m_bindings) {
        m_device.bindVertexBuffer(binding.vertexBuffer.get());

        for (GBufferBinding &bufferBinding : binding.bindings) {
            glEnableVertexAttribArray(bufferBinding.position);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"
            glVertexAttribPointer(
                bufferBinding.position,
                bufferBinding.size,
                toOGLEnum(bufferBinding.type),
                (GLboolean) (bufferBinding.normalized ? GL_TRUE : GL_FALSE),
                bufferBinding.stride,
                (const void *) bufferBinding.offset
            );
#pragma clang diagnostic pop
        }
    }
}

void GVertexBufferBindingsGL20::unbind() {
//    glBindVertexArray(0);
}

void GVertexBufferBindingsGL20::setIndexBuffer(HGIndexBuffer indexBuffer) {
    m_indexBuffer = indexBuffer;
}

void GVertexBufferBindingsGL20::addVertexBufferBinding(GVertexBufferBinding binding) {
    m_bindings.push_back(binding);
}

static int VAO_updated = 0;
void GVertexBufferBindingsGL20::save() {
//    std::cout << "VAO_updated = " << VAO_updated++ << std::endl;

//    m_device.bindVertexBufferBindings(this);
//    for (GVertexBufferBinding &binding : m_bindings) {
//        for (GBufferBinding &bufferBinding : binding.bindings) {
//
//        }
//    }
//    m_device.bindIndexBuffer(nullptr);
//    m_device.bindVertexBuffer(nullptr);



//    m_device.bindVertexBufferBindings(nullptr);

}



