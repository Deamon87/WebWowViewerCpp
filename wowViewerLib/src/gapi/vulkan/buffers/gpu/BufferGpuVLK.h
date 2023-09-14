//
// Created by Deamon on 9/12/2023.
//

#ifndef AWEBWOWVIEWERCPP_BUFFERGPUVLK_H
#define AWEBWOWVIEWERCPP_BUFFERGPUVLK_H

#include <memory>

class GDeviceVLK;
typedef std::shared_ptr<GDeviceVLK> HGDeviceVLK;

#include "../../GDeviceVulkan.h"

class BufferGpuVLK {
public:
    BufferGpuVLK(const HGDeviceVLK &device, int size, VkBufferUsageFlags usageFlags, const char *obj_name);
    ~BufferGpuVLK();

    uint32_t size() {
        return m_size;
    }

    VkBuffer getBuffer() {
        return m_hBuffer;
    }
private:
    HGDeviceVLK m_device;

    VkBuffer m_hBuffer = VK_NULL_HANDLE;
    VmaAllocation m_hBufferAlloc = VK_NULL_HANDLE;
    uint32_t m_size;
};


#endif //AWEBWOWVIEWERCPP_BUFFERGPUVLK_H
