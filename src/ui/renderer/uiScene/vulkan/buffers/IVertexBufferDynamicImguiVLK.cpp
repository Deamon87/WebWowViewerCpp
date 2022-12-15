//
// Created by Deamon on 14.12.22.
//

#include "IVertexBufferDynamicImguiVLK.h"

RVertexBufferDynamicImguiVLK::RVertexBufferDynamicImguiVLK(FrontendUIRenderer &renderer, VmaVirtualAllocation alloc, VkDeviceSize offset, int size) : m_renderer(renderer) {
    m_alloc = alloc;
    m_offset = offset;
    m_size = size;
}

RVertexBufferDynamicImguiVLK::~RVertexBufferDynamicImguiVLK() {

}
