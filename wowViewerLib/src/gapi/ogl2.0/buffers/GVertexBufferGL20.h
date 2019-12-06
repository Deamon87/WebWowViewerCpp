//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GVERTEXBUFFER_H
#define WEBWOWVIEWERCPP_GVERTEXBUFFER_H

#include "../../interface/IDevice.h"
#include "../../interface/buffers/IVertexBuffer.h"
#include "../GDeviceGL20.h"
#include <memory>

class GVertexBufferGL20 : public IVertexBuffer {
    friend class GDeviceGL20;

    explicit GVertexBufferGL20(IDevice &device);
public:
    ~GVertexBufferGL20() override;
private:
    void createBuffer();
    void destroyBuffer();
    void bind() override; //Should be called only by GDevice
    void unbind() override;

public:
    void uploadData(void *, int length) override;

private:
    IDevice &m_device;

private:
    std::vector<char> pIdentifierBuffer {};
    size_t m_size;
    bool m_buffCreated = false;
    bool m_dataUploaded = false;

};

#endif //WEBWOWVIEWERCPP_GVERTEXBUFFER_H
