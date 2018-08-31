//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H
#define WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H

class GVertexBuffer;
class GIndexBuffer;
class GVertexBufferBindings;

#include <vector>
#include <cstdint>
#include "buffers/GVertexBuffer.h"
#include "../interface/IVertexBufferBindings.h"
#include "../interface/IDevice.h"
#include "GDevice.h"

class GVertexBufferBindings : public virtual IVertexBufferBindings {
    friend class GDevice;
private:
    std::vector<GVertexBufferBinding> m_bindings;
    HGIndexBuffer m_indexBuffer = HGIndexBuffer(nullptr);


private:
    IDevice &m_device;
private:
    void * m_buffer;

public:
    explicit GVertexBufferBindings(IDevice &m_device);
    ~GVertexBufferBindings() override;

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
