//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H
#define WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H

class GDevice;

#include <vector>
#include <cstdint>
#include "GVertexBuffer.h"

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
    GVertexBuffer vertexBuffer;
    std::vector<GVertexBufferBinding> bindings;
};

class GVertexBufferBindings {
private:
    std::vector<GVertexBufferBinding> m_bindings;
    GIndexBuffer *m_indexBuffer = nullptr;


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
    void save();

    void setIndexBuffer(GIndexBuffer &indexBuffer);
    void addVertexBufferBinding(GVertexBufferBinding binding);

};


#endif //WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H
