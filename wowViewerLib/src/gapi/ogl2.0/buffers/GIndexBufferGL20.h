//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GINDEXBUFFER20_H
#define WEBWOWVIEWERCPP_GINDEXBUFFER20_H

#include "../../interface/buffers/IBuffer.h"
#include "../../interface/IDevice.h"
#include "../GDeviceGL20.h"


class GIndexBufferGL20 : public IBuffer{
    friend class GDeviceGL20;

    explicit GIndexBufferGL20(IDevice &device);
public:
    ~GIndexBufferGL20() override;

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
    std::vector<char> buffer = {};

    bool m_buffCreated = true;
    size_t m_size;
    bool m_dataUploaded = false;
};


#endif //WEBWOWVIEWERCPP_GINDEXBUFFER_H
