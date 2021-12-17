//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GINDEXBUFFER_H
#define WEBWOWVIEWERCPP_GINDEXBUFFER_H

#include "../../interface/buffers/IIndexBuffer.h"
#include "../../interface/IDevice.h"
#include "../GDeviceGL33.h"


class GIndexBufferGL33 : public IIndexBuffer{
    friend class GDeviceGL33;

    explicit GIndexBufferGL33(const HGDevice &device);
public:
    ~GIndexBufferGL33() override;

private:
    void createBuffer();
    void destroyBuffer();
    void bind();
    void unbind();

public:
    void uploadData(void *, int length) override;

private:
    const HGDevice m_device;

private:
    std::vector<char> buffer = {};

    bool m_buffCreated = true;
    size_t m_size;
    bool m_dataUploaded = false;
};


#endif //WEBWOWVIEWERCPP_GINDEXBUFFER_H
