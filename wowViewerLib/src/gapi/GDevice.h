//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GDEVICE_H
#define WEBWOWVIEWERCPP_GDEVICE_H

#include "GIndexBuffer.h"
#include "GVertexBuffer.h"
#include "GVertexBufferBindings.h"

class GDevice {
public:
    void bindIndexBuffer(GIndexBuffer *buffer);
    void bindVertexBuffer(GVertexBuffer *buffer);
    void bindVertexBufferBindings(GVertexBufferBindings *buffer);


private:
    GIndexBuffer *m_lastBindIndexBuffer;
    GVertexBuffer *m_lastBindVertexBuffer;
    GVertexBufferBindings *m_vertexBufferBindings;
};

#endif //WEBWOWVIEWERCPP_GDEVICE_H
