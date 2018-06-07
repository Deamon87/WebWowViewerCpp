//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GDEVICE_H
#define WEBWOWVIEWERCPP_GDEVICE_H

#include "GIndexBuffer.h"
#include "GVertexBuffer.h"

class GDevice {
public:
    void bindIndexBuffer(GIndexBuffer *buffer);
    void bindVertexBuffer(GVertexBuffer *buffer);
private:
    GIndexBuffer *m_lastBindIndexBuffer;
    GVertexBuffer *m_lastBindVertexBuffer;

};

#endif //WEBWOWVIEWERCPP_GDEVICE_H
