//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GDEVICE_H
#define WEBWOWVIEWERCPP_GDEVICE_H

#include "../engine/opengl/header.h"
#include "GIndexBuffer.h"
#include "GVertexBuffer.h"

class GDevice {


public:
    void bindIndexBuffer(GIndexBuffer *buffer) {
        if (buffer != m_lastBindIndexBuffer) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->buffer);
            m_lastBindIndexBuffer = buffer;
        }
    }

    void bindVertexBuffer(GVertexBuffer *buffer) {
        if (buffer != m_lastBindVertexBuffer) {
            glBindBuffer(GL_ARRAY_BUFFER, buffer->buffer);
            m_lastBindVertexBuffer = buffer;
        }
    }




private:
    GIndexBuffer *m_lastBindIndexBuffer;
    GVertexBuffer *m_lastBindVertexBuffer;

};


#endif //WEBWOWVIEWERCPP_GDEVICE_H
