//
// Created by deamon on 05.06.18.
//

#include "../engine/opengl/header.h"
#include "GDevice.h"

struct BlendModeDesc {
    bool blendModeEnable;
    GLuint SrcColor;
    GLuint DestColor;
    GLuint SrcAlpha;
    GLuint DestAlpha;
};

BlendModeDesc blendModes[(int)EGxBlendEnum::GxBlend_MAX] = {
        {false,GL_ONE,GL_ZERO,GL_ONE,GL_ZERO},
        {false,GL_ONE,GL_ZERO,GL_ONE,GL_ZERO},
        {true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA},
        {true,GL_SRC_ALPHA,GL_ONE,GL_ZERO,GL_ONE},
        {true,GL_DST_COLOR,GL_ZERO,GL_DST_ALPHA,GL_ZERO},
        {true,GL_DST_COLOR,GL_SRC_COLOR,GL_DST_ALPHA,GL_SRC_ALPHA},
        {true,GL_DST_COLOR,GL_ONE,GL_DST_ALPHA,GL_ONE},
        {true,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE},
        {true,GL_ONE_MINUS_SRC_ALPHA,GL_ZERO,GL_ONE_MINUS_SRC_ALPHA,GL_ZERO},
        {true,GL_SRC_ALPHA,GL_ZERO,GL_SRC_ALPHA,GL_ZERO},
        {true,GL_CONSTANT_ALPHA,GL_ONE_MINUS_CONSTANT_ALPHA,GL_CONSTANT_ALPHA,GL_ONE_MINUS_CONSTANT_ALPHA},
        {true,GL_ONE_MINUS_DST_COLOR,GL_ONE,GL_ONE,GL_ZERO},
        {true,GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA}
};

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

