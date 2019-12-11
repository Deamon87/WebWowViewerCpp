//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GVERTEXBUFFERDYNAMICVLK_H
#define WEBWOWVIEWERCPP_GVERTEXBUFFERDYNAMICVLK_H

#include "../../interface/IDevice.h"
#include "../../interface/buffers/IVertexBuffer.h"
#include "../../interface/buffers/IVertexBufferDynamic.h"
#include "../GDeviceVulkan.h"
#include <memory>

class GVertexBufferDynamicVLK : public IVertexBufferDynamic {
    friend class GDeviceVLK;

    explicit GVertexBufferDynamicVLK(IDevice &device, size_t maxSize);
public:
    ~GVertexBufferDynamicVLK() override;

    void *getPointerForModification() override;

    void save(size_t sizeToSave) override;
    void resize(size_t sizeToSave) override;
private:
    void createBuffer();
    void destroyBuffer();
    void bind() override; //Should be called only by GDevice
    void unbind();

public:
    void uploadData(void *, int length) override;

private:
    GDeviceVLK &m_device;
    VkBuffer g_hVertexBuffer;
    VmaAllocation g_hVertexBufferAlloc;

    VkBuffer stagingVertexBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingVertexBufferAlloc = VK_NULL_HANDLE;
    VmaAllocationInfo stagingVertexBufferAllocInfo;
private:
    size_t m_size = 0;
    bool m_buffCreated = false;
    bool m_dataUploaded = false;
};

#endif //WEBWOWVIEWERCPP_GVERTEXBUFFERDYNAMICVLK_H
