//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H
#define WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H

class GVertexBufferGL33;
class GIndexBufferGL33;
class GVertexBufferBindingsVLK;

#include <vector>
#include <cstdint>
#include "../interface/IVertexBufferBindings.h"
#include "../interface/IDevice.h"
#include "GDeviceVulkan.h"


struct vkBufferFormatHolder {
    VkVertexInputBindingDescription bindingDescription;
    std::vector<VkVertexInputAttributeDescription> attributeDescription;
};

class GVertexBufferBindingsVLK : public IVertexBufferBindings {
    friend class GDeviceVLK;
private:
    std::vector<GVertexBufferBinding> m_bindings;
    std::vector<vkBufferFormatHolder> m_BufferBindingsVLK;
    HGIndexBuffer m_indexBuffer = HGIndexBuffer(nullptr);


private:
    IDevice &m_device;
private:


public:
    explicit GVertexBufferBindingsVLK(IDevice &m_device);
    ~GVertexBufferBindingsVLK() override;

private:

public:
    void save() override;

    void setIndexBuffer(HGIndexBuffer indexBuffer) override;
    void addVertexBufferBinding(GVertexBufferBinding binding) override;
    std::vector<vkBufferFormatHolder> &getVLKFormat();

};

#endif //WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H
