//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H
#define WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H

class GVertexBufferGL20;
class GIndexBufferGL20;
class GVertexBufferBindingsGL20;

#include <vector>
#include <cstdint>
#include "buffers/GVertexBufferGL20.h"
#include "../interface/IVertexBufferBindings.h"
#include "../interface/IDevice.h"
#include "GDeviceGL20.h"

class GVertexBufferBindingsGL20 : public IVertexBufferBindings {
    friend class GDeviceGL20;
private:
    std::vector<GVertexBufferBinding> m_bindings;
    HGIndexBuffer m_indexBuffer = HGIndexBuffer(nullptr);


private:
    IDevice &m_device;
private:
    std::vector<char> m_buffer;

public:
    explicit GVertexBufferBindingsGL20(IDevice &m_device);
    ~GVertexBufferBindingsGL20() override;

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
