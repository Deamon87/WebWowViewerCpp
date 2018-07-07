//
// Created by deamon on 05.06.18.
//

#include "../engine/opengl/header.h"
#include "GDevice.h"
#include "../engine/algorithms/hashString.h"


void GDevice::bindIndexBuffer(GIndexBuffer *buffer) {
    if (buffer == nullptr ) {
        if (m_lastBindIndexBuffer == nullptr) {
            m_lastBindIndexBuffer->unbind();
            m_lastBindIndexBuffer = nullptr;
        }
    } else if (buffer != m_lastBindIndexBuffer) {
        buffer->bind();
        m_lastBindIndexBuffer = buffer;
    }
}

void GDevice::bindVertexBuffer(GVertexBuffer *buffer)  {
    if (buffer == nullptr) {
        if (m_lastBindVertexBuffer != nullptr) {
            m_lastBindVertexBuffer->unbind();
            m_lastBindVertexBuffer = nullptr;
        }
    }  else if (buffer != m_lastBindVertexBuffer) {
        buffer->bind();
        m_lastBindVertexBuffer = buffer;
    }
}

void GDevice::bindUniformBuffer(GUniformBuffer *buffer, int slot)  {
    if (buffer == nullptr) {
        if (m_uniformBuffer[slot] != nullptr) {
            m_uniformBuffer[slot]->unbind();
            m_uniformBuffer[slot] = nullptr;
        }
    }  else if (buffer != m_uniformBuffer[slot]) {
        buffer->bind();
        m_uniformBuffer[slot] = buffer;
    }
}

void GDevice::bindVertexBufferBindings(GVertexBufferBindings *buffer) {
    if (buffer == nullptr) {
       if (m_vertexBufferBindings != nullptr) {
           m_vertexBufferBindings->unbind();
       }
        m_lastBindIndexBuffer = nullptr;
        m_lastBindVertexBuffer = nullptr;
    } else {
        bindIndexBuffer(nullptr);
        bindVertexBuffer(nullptr);

        if (buffer != m_vertexBufferBindings) {
            buffer->bind();
            m_vertexBufferBindings = buffer;
        }
    }
}

std::shared_ptr<GShaderPermutation> GDevice::getShader(std::string shaderName) {
    const char * cstr = shaderName.c_str();
    size_t hash = CalculateFNV(cstr);
    if (m_shaderPermutCache.count(hash) > 0) {

        std::shared_ptr<GShaderPermutation> ptr =  m_shaderPermutCache.at(hash);
        return ptr;
    }

    std::shared_ptr<GShaderPermutation> sharedPtr = std::make_shared<GShaderPermutation>(GShaderPermutation(shaderName, *this));

    m_shaderPermutCache[hash] = sharedPtr;

    return m_shaderPermutCache[hash];
}

GUniformBuffer &GDevice::createUniformBuffer() {
    std::shared_ptr<GUniformBuffer> h_uniformBuffer;
    h_uniformBuffer.reset(new GUniformBuffer(*this, 0));

    std::weak_ptr<GUniformBuffer> w_uniformBuffer = h_uniformBuffer;

    m_unfiormBufferCache.push_back(w_uniformBuffer);
}

void GDevice::drawMeshes(std::vector<GMesh *> &meshes) {

}

