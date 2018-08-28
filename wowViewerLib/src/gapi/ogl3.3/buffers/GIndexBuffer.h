//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GINDEXBUFFER_H
#define WEBWOWVIEWERCPP_GINDEXBUFFER_H

class GDevice;
#include "../GDevice.h"
#include "../../interface/buffers/IIndexBuffer.h"

class GIndexBuffer : public IIndexBuffer{
    friend class GDevice;

    explicit GIndexBuffer(GDevice &device);
public:
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

    bool m_buffCreated = true;
    size_t m_size;
    bool m_dataUploaded = false;
};


#endif //WEBWOWVIEWERCPP_GINDEXBUFFER_H
