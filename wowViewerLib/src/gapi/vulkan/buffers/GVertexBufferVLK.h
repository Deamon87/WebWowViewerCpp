//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GVERTEXBUFFER_H
#define WEBWOWVIEWERCPP_GVERTEXBUFFER_H

#include "../../interface/IDevice.h"
#include "../../interface/buffers/IVertexBuffer.h"
#include "../GDeviceVulkan.h"
#include <memory>

class GVertexBufferVLK : public IVertexBuffer {
    friend class GDeviceVLK;

    explicit GVertexBufferVLK(IDevice &device);
public:
    ~GVertexBufferVLK() override;
private:
    void createBuffer();
    void destroyBuffer();
    void bind(); //Should be called only by GDevice
    void unbind();

public:
    void uploadData(void *, int length) override;

private:
    GDeviceVLK &m_device;
    VkBuffer g_hVertexBuffer;
    VmaAllocation g_hVertexBufferAlloc;

    VkBuffer stagingVertexBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingVertexBufferAlloc = VK_NULL_HANDLE;


private:

    size_t m_size;
    bool m_buffCreated = false;
    bool m_dataUploaded = false;

};

#endif //WEBWOWVIEWERCPP_GVERTEXBUFFER_H
