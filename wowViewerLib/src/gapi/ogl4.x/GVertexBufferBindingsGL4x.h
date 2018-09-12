//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_4X_H
#define WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_4X_H

class GVertexBufferGL4x;
class GIndexBufferGL4x;
class GVertexBufferBindingsGL4x;

#include <vector>
#include <cstdint>
#include "buffers/GVertexBufferGL4x.h"
#include "../interface/IVertexBufferBindings.h"
#include "../interface/IDevice.h"
#include "GDeviceGL4x.h"

class GVertexBufferBindingsGL4x : public IVertexBufferBindings {
    friend class GDeviceGL4x;
private:
    std::vector<GVertexBufferBinding> m_bindings;
    HGIndexBuffer m_indexBuffer = HGIndexBuffer(nullptr);


private:
    IDevice &m_device;
private:
    void * m_buffer;

public:
    explicit GVertexBufferBindingsGL4x(IDevice &m_device);
    ~GVertexBufferBindingsGL4x() override;

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

#endif //WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_4X_H
