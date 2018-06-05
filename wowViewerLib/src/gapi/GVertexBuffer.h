//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GVERTEXBUFFER_H
#define WEBWOWVIEWERCPP_GVERTEXBUFFER_H

class GDevice;
#include "../engine/opengl/header.h"
#include "GDevice.h"

class GVertexBuffer {
    friend class GDevice;

    GVertexBuffer(GDevice &device) : m_device(device) {
            createBuffer();
    }
    ~GIndexBuffer(){
        destroyBuffer();
    }
private:
    void createBuffer();
    void destroyBuffer();

public:
    void uploadData(void *, int length);

private:
    GDevice &m_device;

private:
    GLuint buffer = 0;
};


#endif //WEBWOWVIEWERCPP_GVERTEXBUFFER_H
