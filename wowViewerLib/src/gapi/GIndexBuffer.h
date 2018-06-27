//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GINDEXBUFFER_H
#define WEBWOWVIEWERCPP_GINDEXBUFFER_H

class GDevice;
#include "GDevice.h"

class GIndexBuffer {
    friend class GDevice;

    explicit GIndexBuffer(GDevice &device);
    ~GIndexBuffer();

private:
    void createBuffer();
    void destroyBuffer();
    void bind();
    void unbind();

public:
    void uploadData(void *, int length);

private:
    GDevice &m_device;

private:
    void * buffer = nullptr;
};


#endif //WEBWOWVIEWERCPP_GINDEXBUFFER_H
