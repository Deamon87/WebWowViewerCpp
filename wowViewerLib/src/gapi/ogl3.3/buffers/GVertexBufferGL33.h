//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GVERTEXBUFFER_H
#define WEBWOWVIEWERCPP_GVERTEXBUFFER_H

#include "../../interface/IDevice.h"
#include "../../interface/buffers/IVertexBuffer.h"
#include "../GDeviceGL33.h"
#include <memory>

class GVertexBufferGL33 : public IVertexBuffer {
    friend class GDeviceGL33;

    explicit GVertexBufferGL33(IDevice &device);
public:
    ~GVertexBufferGL33() override;
private:
    void createBuffer();
    void destroyBuffer();
    void bind(); //Should be called only by GDevice
    void unbind();

public:
    void uploadData(void *, int length) override;

private:
    IDevice &m_device;

private:
    void * pIdentifierBuffer = nullptr;
    size_t m_size;
    bool m_buffCreated = false;
    bool m_dataUploaded = false;

};

#endif //WEBWOWVIEWERCPP_GVERTEXBUFFER_H
