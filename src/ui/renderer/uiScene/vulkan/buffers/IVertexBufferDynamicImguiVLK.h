//
// Created by Deamon on 14.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IVERTEXBUFFERDYNAMICIMGUIVLK_H
#define AWEBWOWVIEWERCPP_IVERTEXBUFFERDYNAMICIMGUIVLK_H

#include "../../buffers/IVertexBufferDynamicImgui.h"
#include "../../FrontendUIRenderer.h"
#include "../../../../../../wowViewerLib/src/gapi/vulkan/vk_mem_alloc.h"

class RVertexBufferDynamicImguiVLK : public IVertexBufferDynamicImgui {
    friend class FrontendUIRenderer;
private:
    VmaVirtualAllocation m_alloc;
    VkDeviceSize m_offset;
    int m_size;

    FrontendUIRenderer &m_renderer;
public:
    RVertexBufferDynamicImguiVLK(FrontendUIRenderer &renderer, VmaVirtualAllocation alloc, VkDeviceSize offset, int size);
    ~RVertexBufferDynamicImguiVLK() override;

    const std::vector<ImDrawVert> &getBuffer() override {
        return stagingVertexBuffer;
    };
    void save(size_t sizeToSave) override {
        m_renderer.planUpload()
    };
};


#endif //AWEBWOWVIEWERCPP_IVERTEXBUFFERDYNAMICIMGUIVLK_H
