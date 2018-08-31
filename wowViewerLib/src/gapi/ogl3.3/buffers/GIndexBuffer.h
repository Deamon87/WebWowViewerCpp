//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GINDEXBUFFER_H
#define WEBWOWVIEWERCPP_GINDEXBUFFER_H

class GDevice;
#include "../GDevice.h"
#include "../../interface/buffers/IIndexBuffer.h"
#include "../../interface/IDevice.h"

class GIndexBuffer : public virtual IIndexBuffer{
    friend class GDevice;

    explicit GIndexBuffer(IDevice &device);
public:
    ~GIndexBuffer() override;

private:
    void createBuffer();
    void destroyBuffer();
    void bind();
    void unbind();

public:
    void uploadData(void *, int length) override;

private:
    IDevice &m_device;

private:
    void * buffer = nullptr;

    bool m_buffCreated = true;
    size_t m_size;
    bool m_dataUploaded = false;
};


#endif //WEBWOWVIEWERCPP_GINDEXBUFFER_H
