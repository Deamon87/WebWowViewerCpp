//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GINDEXBUFFER_H
#define WEBWOWVIEWERCPP_GINDEXBUFFER_H

class GDevice;
#include "../engine/opengl/header.h"
#include "GDevice.h"

class GIndexBuffer {
    friend class GDevice;

    GIndexBuffer(GDevice &device) : m_device(device) {
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


#endif //WEBWOWVIEWERCPP_GINDEXBUFFER_H
