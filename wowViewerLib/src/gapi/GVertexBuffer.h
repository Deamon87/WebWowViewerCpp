//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GVERTEXBUFFER_H
#define WEBWOWVIEWERCPP_GVERTEXBUFFER_H

class GDevice;
#include "GDevice.h"

class GVertexBuffer {
    friend class GDevice;

    explicit GVertexBuffer(GDevice &device);
    ~GVertexBuffer();
private:
    void createBuffer();
    void destroyBuffer();
    void bind(); //Should be called only by GDevice
    void unbind();

public:
    void uploadData(void *, int length);

private:
    GDevice &m_device;



private:
    void * pIdentifierBuffer = nullptr;

};


#endif //WEBWOWVIEWERCPP_GVERTEXBUFFER_H
