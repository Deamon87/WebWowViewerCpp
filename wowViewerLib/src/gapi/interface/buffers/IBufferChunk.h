    //
// Created by deamon on 09.12.19.
//

#ifndef AWEBWOWVIEWERCPP_IBUFFERCHUNK_H
#define AWEBWOWVIEWERCPP_IBUFFERCHUNK_H

#include <functional>
#include <assert.h>
#include "IBuffer.h"
#include "../../../renderer/mapScene/FrameDependentData.h"

template<typename T>
class IBufferChunk {
public:
    using structureType = T;

    virtual ~IBufferChunk() = default;
    virtual T &getObject() = 0;
    virtual void save() = 0;
};
#endif //AWEBWOWVIEWERCPP_IBUFFERCHUNK_H
