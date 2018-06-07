//
// Created by deamon on 05.06.18.
//

#include "../engine/opengl/header.h"
#include "GVertexBufferBindings.h"

GVertexBufferBindings::GVertexBufferBindings(GDevice &m_device) : m_device(m_device) {
    m_buffer = new GLuint;
    createBuffer();
}

GVertexBufferBindings::~GVertexBufferBindings() {
    destroyBuffer();
}

void GVertexBufferBindings::createBuffer() {
    glGenVertexArrays(1, (GLuint *)this->m_buffer);
}

void GVertexBufferBindings::destroyBuffer() {
    glDeleteVertexArrays(1, (GLuint *)this->m_buffer);
}

void GVertexBufferBindings::bind() {
    glBindVertexArray(*(GLuint *)this->m_buffer);
}

void GVertexBufferBindings::unbind() {
    glBindVertexArray(0);
}

void GVertexBufferBindings::setIndexBuffer(GIndexBuffer &indexBuffer) {
    m_indexBuffer = &indexBuffer;
}

void GVertexBufferBindings::addVertexBufferBinding(GVertexBufferBinding binding) {
    m_bindings.push_back(binding);
}

void GVertexBufferBindings::save() {

}



