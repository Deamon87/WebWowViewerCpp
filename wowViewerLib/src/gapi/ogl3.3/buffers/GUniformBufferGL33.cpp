//
// Created by Deamon on 6/30/2018.
//
#include <memory.h>
#include <iostream>
#include "../../../engine/opengl/header.h"
#include "GUniformBufferGL33.h"
#include "../../interface/IDevice.h"

GUniformBufferGL33::GUniformBufferGL33(IDevice &device, size_t size) : m_device(device){
    m_size = size;
    pFrameOneContent = new char[size];
    pFrameTwoContent = new char[size];
//    createBuffer();
}

GUniformBufferGL33::~GUniformBufferGL33() {
    if (m_buffCreated) {
        destroyBuffer();
    }

    delete (char *)pFrameOneContent;
    delete (char *)pFrameTwoContent;
}

void GUniformBufferGL33::createBuffer() {
    glGenBuffers(1, (GLuint *) &glBuffId[0]);
    m_buffCreated = true;

}

void GUniformBufferGL33::destroyBuffer() {
    glDeleteBuffers(1, (GLuint *)&glBuffId[0]);
}
void GUniformBufferGL33::bind(int bindingPoint) { //Should be called only by GDevice
    if (m_buffCreated && bindingPoint == -1) {
        glBindBuffer(GL_UNIFORM_BUFFER, this->glBuffId[0]);
    } else if (m_buffCreated) {
        glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, this->glBuffId[0]);
    } else {
        glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, getIdentifierBuffer(), m_offset[m_device.getDrawFrameNumber()], m_size);
    }
}
void GUniformBufferGL33::unbind() {
//    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GUniformBufferGL33::uploadData(void * data, int length) {
    m_device.bindVertexUniformBuffer(this, -1);

    assert(m_buffCreated);
    assert(length > 0 && length <= 65536);

    if (!m_dataUploaded) {
        glBufferData(GL_UNIFORM_BUFFER, length, nullptr, GL_DYNAMIC_DRAW);
    } else {
        glBufferSubData(GL_UNIFORM_BUFFER, 0, length, data);
    }

    m_size = (size_t) length;

    m_dataUploaded = true;
    m_needsUpdate = false;
//    m_device.bindVertexUniformBuffer(this, -1);
}

void GUniformBufferGL33::save(bool initialSave) {
//    if (memcmp(pPreviousContent, pContent, m_size) != 0) {
        //1. Copy new to prev
        m_needsUpdate = true;

//        2. Update UBO
        if (m_buffCreated) {
            void * data = getPointerForModification();
            this->uploadData(data, m_size);
        }
//    }
}

void *GUniformBufferGL33::getPointerForUpload() {
//    if (!m_device.getIsEvenFrame()) {
//        return pFrameTwoContent;
//    } else {
        return pFrameOneContent;
//    }
}

void *GUniformBufferGL33::getPointerForModification() {
//    if (m_device.getIsEvenFrame()) {
//        return pFrameTwoContent;
//    } else {
        return pFrameOneContent;
//    }
}

void GUniformBufferGL33::setUpdateHandler(std::function<void(IUniformBuffer* self)> handler){
    m_handler = handler;
}
void GUniformBufferGL33::update() {
    if (m_handler) {
        m_handler(this);
    }
}