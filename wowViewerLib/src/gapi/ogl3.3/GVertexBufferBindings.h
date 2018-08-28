//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H
#define WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H

class GDevice;
class GVertexBuffer;
class GIndexBuffer;
class GVertexBufferBindings;

#include <vector>
#include <cstdint>
#include "buffers/GVertexBuffer.h"

#include "GDevice.h"

struct GBufferBinding{
    uint32_t position;
    uint32_t size;
    uint32_t type;
    bool normalized;
    uint32_t stride;
    uint32_t offset;
};

struct GVertexBufferBinding {
    HGVertexBuffer vertexBuffer;
    std::vector<GBufferBinding> bindings;
};




class GVertexBufferBindings {
    friend class GDevice;
private:
    std::vector<GVertexBufferBinding> m_bindings;
    HGIndexBuffer m_indexBuffer = HGIndexBuffer(nullptr);


private:
    GDevice &m_device;
private:
    void * m_buffer;

public:
    GVertexBufferBindings(GDevice &m_device);
    ~GVertexBufferBindings();

private:
    void createBuffer();
    void destroyBuffer();
    void bind(); //Should be called only by GDevice
    void unbind();
public:
    void save();

    void setIndexBuffer(HGIndexBuffer indexBuffer);
    void addVertexBufferBinding(GVertexBufferBinding binding);

};

#endif //WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H
