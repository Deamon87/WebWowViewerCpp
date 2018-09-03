//
// Created by Deamon on 8/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_IVERTEXBUFFERBINDINGS_H
#define AWEBWOWVIEWERCPP_IVERTEXBUFFERBINDINGS_H

#include "IDevice.h"

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

class IVertexBufferBindings {
public:
    virtual ~IVertexBufferBindings() {};
public:
    virtual void save() = 0;

    virtual void setIndexBuffer(HGIndexBuffer indexBuffer) = 0;
    virtual void addVertexBufferBinding(GVertexBufferBinding binding) = 0;
};

#endif //AWEBWOWVIEWERCPP_IVERTEXBUFFERBINDINGS_H
