//
// Created by Deamon on 24.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IBUFFERVLK_H
#define AWEBWOWVIEWERCPP_IBUFFERVLK_H

#include <vulkan/vulkan.h>
#include "../../interface/buffers/IBuffer.h"
#include "../bindable/DSBindable.h"

class IBufferVLK : public IBuffer, public IDSBindable {
public:
    IBufferVLK() : IDSBindable(false) {};

    virtual VkBuffer getGPUBuffer() = 0;
    virtual size_t getOffset() = 0;
};


#endif //AWEBWOWVIEWERCPP_IBUFFERVLK_H
