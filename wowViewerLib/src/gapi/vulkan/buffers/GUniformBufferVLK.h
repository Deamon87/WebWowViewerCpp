//
// Created by Deamon on 6/30/2018.
//

#ifndef AWEBWOWVIEWERCPP_GUNIFORMBUFFER_H
#define AWEBWOWVIEWERCPP_GUNIFORMBUFFER_H


#include <cstdio>
#include <cassert>
#include "../GDeviceVulkan.h"
#include "../../interface/buffers/IUniformBuffer.h"

class GUniformBufferVLK : public IUniformBuffer {
public:
    friend class GDeviceVLK;
    friend class GMeshVLK;
    friend class GShaderPermutationVLK;

    explicit GUniformBufferVLK(IDevice &device, size_t size);
    ~GUniformBufferVLK() override;

    void createBuffer() override;

    size_t getSize() {return m_size;}
private:
    void destroyBuffer();
    void bind(int bindingPoint); //Should be called only by GDevice
    void unbind();

private:
    void uploadData(void * data, int length);
    void uploadFromStaging(int length);
    void resize(int newLength);

private:
    GDeviceVLK *m_device;

private:
    size_t m_size;
    bool m_buffCreated = false;
    bool m_dataUploaded = false;

    VkBuffer g_buf;
    VmaAllocation g_alloc;
    VmaAllocationInfo g_allocInfo;

    VkBuffer stagingUBOBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingUBOBufferAlloc = VK_NULL_HANDLE;
    VmaAllocationInfo stagingUBOBufferAllocInfo = {};
};


#endif //AWEBWOWVIEWERCPP_GUNIFORMBUFFER_H
