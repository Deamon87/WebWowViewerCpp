//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GINDEXBUFFER_H
#define WEBWOWVIEWERCPP_GINDEXBUFFER_H

#include "../../interface/buffers/IIndexBuffer.h"
#include "../../interface/IDevice.h"
#include "../GDeviceVulkan.h"


class GIndexBufferVLK : public IIndexBuffer{
    friend class GDeviceVLK;

    explicit GIndexBufferVLK(IDevice &device);
public:
    ~GIndexBufferVLK() override;

private:
    void createBuffer();
    void destroyBuffer();
    void bind();
    void unbind();

public:
    void uploadData(void *, int length) override;

private:
    GDeviceVLK &m_device;
    VkBuffer g_hIndexBuffer;
    VmaAllocation g_hIndexBufferAlloc;

    VkBuffer stagingIndexBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingIndexBufferAlloc = VK_NULL_HANDLE;

private:
    size_t m_size;
    bool m_buffCreated = false;
    bool m_dataUploaded = false;
};


#endif //WEBWOWVIEWERCPP_GINDEXBUFFER_H
