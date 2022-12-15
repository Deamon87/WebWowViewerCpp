//
// Created by Deamon on 12.12.22.
//

#ifndef AWEBWOWVIEWERCPP_FRONTENDUIRENDERFORWARDVLK_H
#define AWEBWOWVIEWERCPP_FRONTENDUIRENDERFORWARDVLK_H


#include <queue>
#include "../FrontendUIRenderer.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/GDeviceVulkan.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

class FrontendUIRenderForwardVLK : public FrontendUIRenderer {
public:
    explicit FrontendUIRenderForwardVLK(HGDeviceVLK hDevice);

    void putIntoQueue(std::shared_ptr<FrontendUIInputParams> &frameInputParams) override;
    void render();

    HIVertexBufferDynamicImgui allocateDynamicVertexBuffer(int size) override;
//    HGVertexBufferDynamic allocateDynamicIndexBuffer() = 0;
private:
    HGDeviceVLK m_device;

    //Frame counter
    int m_frame = 0;

    //Buffers
    struct Buffer {
        VkBuffer g_hBuffer = VK_NULL_HANDLE;
        VmaAllocation g_hBufferAlloc = VK_NULL_HANDLE;

        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        VmaAllocation stagingBufferAlloc = VK_NULL_HANDLE;
        VmaAllocationInfo stagingBufferAllocInfo;

        //Virtual block for suballocations
        VmaVirtualBlock virtualBlock;
    };

    std::array<Buffer, MAX_FRAMES_IN_FLIGHT> vertexBuffers; //Both vertexes here are of exact size
    std::array<Buffer, MAX_FRAMES_IN_FLIGHT> indexBuffers;

    //Rendering pipelining
    std::mutex m_inputParamsMtx;
    std::queue<std::shared_ptr<FrontendUIInputParams>> m_inputParams;
    std::queue<std::shared_ptr<FrontendUIInputParams>> m_updateParams;

private:
    void createBuffer(Buffer &buffer, int bufferSize, VkBufferUsageFlags bufferUsageFlags);
    void destroyBuffer(Buffer &buffer);

};


#endif //AWEBWOWVIEWERCPP_FRONTENDUIRENDERFORWARDVLK_H
