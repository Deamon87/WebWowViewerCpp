//
// Created by deamon on 05.06.18.
//

#include <iostream>
#include "GVertexBufferBindingsVLK.h"
#include "../interface/IDevice.h"

GVertexBufferBindingsVLK::GVertexBufferBindingsVLK(IDevice &m_device) : m_device(m_device) {

}

GVertexBufferBindingsVLK::~GVertexBufferBindingsVLK() {
    destroyBuffer();
}

void GVertexBufferBindingsVLK::createBuffer() {
}

void GVertexBufferBindingsVLK::destroyBuffer() {
}

void GVertexBufferBindingsVLK::bind() {
}

void GVertexBufferBindingsVLK::unbind() {
}

void GVertexBufferBindingsVLK::setIndexBuffer(HGIndexBuffer indexBuffer) {
    m_indexBuffer = indexBuffer;
}

void GVertexBufferBindingsVLK::addVertexBufferBinding(GVertexBufferBinding binding) {
    m_bindings.push_back(binding);
}

static int VAO_updated = 0;
void GVertexBufferBindingsVLK::save() {


}



