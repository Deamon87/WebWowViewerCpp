//
// Created by Deamon on 9/12/2023.
//

#ifndef AWEBWOWVIEWERCPP_BUFFERSTAGINGVLK_H
#define AWEBWOWVIEWERCPP_BUFFERSTAGINGVLK_H

#include "../../GDeviceVulkan.h"

class BufferStagingVLK {
public:
    BufferStagingVLK(const HGDeviceVLK &device, int size);
    ~BufferStagingVLK();
private:
    HGDeviceVLK m_device;

    VkBuffer m_stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation m_stagingBufferAlloc = VK_NULL_HANDLE;
    VmaAllocationInfo m_stagingBufferAllocInfo;
public:
    void *getPointer();
    VkBuffer getBuffer() {return m_stagingBuffer;};
};


#endif //AWEBWOWVIEWERCPP_BUFFERSTAGINGVLK_H
