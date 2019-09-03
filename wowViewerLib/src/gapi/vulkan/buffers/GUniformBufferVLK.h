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

    explicit GUniformBufferVLK(IDevice &device, size_t size);
    ~GUniformBufferVLK() override;

    void *getPointerForModification() override;
    void *getPointerForUpload() override;

    void save(bool initialSave = false) override;
    void createBuffer() override;

    void commitUpload();
private:

    void destroyBuffer();
    void bind(int bindingPoint); //Should be called only by GDevice
    void unbind();

private:
    void uploadData(void * data, int length);

private:
    GDeviceVLK *m_device;

private:
    size_t m_size;
    size_t m_offset[4] = {0, 0, 0, 0};

    void * pFrameOneContent;
    bool m_buffCreated = false;
    bool m_dataUploaded = false;

    int m_creationIndex = 0;

    bool m_needsUpdate = false;

    VkBuffer g_buf[4];
    VmaAllocation g_alloc[4];
    VmaAllocationInfo g_allocInfo[4];


    VkBuffer stagingUBOBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingUBOBufferAlloc = VK_NULL_HANDLE;
    VmaAllocationInfo stagingUBOBufferAllocInfo = {};
};


#endif //AWEBWOWVIEWERCPP_GUNIFORMBUFFER_H
