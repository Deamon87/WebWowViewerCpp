//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GINDEXBUFFER_4X_H
#define WEBWOWVIEWERCPP_GINDEXBUFFER_4X_H

#include "../../interface/buffers/IIndexBuffer.h"
#include "../../interface/IDevice.h"
#include "../GDeviceGL4x.h"


class GIndexBufferGL4x : public IIndexBuffer{
    friend class GDeviceGL4x;

    explicit GIndexBufferGL4x(IDevice &device);
public:
    ~GIndexBufferGL4x() override;

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

    bool m_buffCreated = false;
    bool m_isImmutable = true;
    size_t m_size;
    bool m_dataUploaded = false;
};


#endif //WEBWOWVIEWERCPP_GINDEXBUFFER_4X_H
