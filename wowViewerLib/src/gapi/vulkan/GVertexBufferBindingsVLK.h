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
#include "buffers/GVertexBufferVLK.h"
#include "../interface/IVertexBufferBindings.h"
#include "../interface/IDevice.h"
#include "GDeviceVulkan.h"

class GVertexBufferBindingsVLK : public IVertexBufferBindings {
    friend class GDeviceVLK;
private:
    std::vector<GVertexBufferBinding> m_bindings;
    HGIndexBuffer m_indexBuffer = HGIndexBuffer(nullptr);


private:
    IDevice &m_device;
private:


public:
    explicit GVertexBufferBindingsVLK(IDevice &m_device);
    ~GVertexBufferBindingsVLK() override;

private:
    void createBuffer();
    void destroyBuffer();
    void bind(); //Should be called only by GDevice
    void unbind();
public:
    void save() override;

    void setIndexBuffer(HGIndexBuffer indexBuffer) override;
    void addVertexBufferBinding(GVertexBufferBinding binding) override;

};

#endif //WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H
