//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H
#define WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H

#include <vector>
#include "GVertexBuffer.h"

struct GVertexBufferBinding{
    uint32_t position;
    uint32_t size;
    uint32_t type;
    bool normalized;
    uint32_t stride;
    uint32_t offset;
};

class GVertexBufferBindings {

private:

    std::vector<GVertexBufferBinding> bindings;


};


#endif //WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H
