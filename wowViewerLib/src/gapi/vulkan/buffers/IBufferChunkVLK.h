//
// Created by Deamon on 30.01.23.
//

#ifndef AWEBWOWVIEWERCPP_IBUFFERCHUNKVLK_H
#define AWEBWOWVIEWERCPP_IBUFFERCHUNKVLK_H


#include "../../interface/buffers/IBufferChunk.h"
#include "GBufferVLK.h"

template<typename T>
class CBufferChunkVLK : public IBufferChunk<T> {
public:
    CBufferChunkVLK(const std::shared_ptr<GBufferVLK> &mainBuffer) {
        subBuffer = mainBuffer->getSubBuffer(sizeof(T));
    }

    T &getObject() override {
        return *(T*)subBuffer->getPointer();
    };
    void save() override {
        subBuffer->save(sizeof(T));
    };

    const std::shared_ptr<IBufferVLK> getSubBuffer() {
        return subBuffer;
    }
private:
    std::shared_ptr<IBufferVLK> subBuffer = nullptr;

};


#endif //AWEBWOWVIEWERCPP_IBUFFERCHUNKVLK_H
