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
    CBufferChunkVLK(const std::shared_ptr<GBufferVLK> &mainBuffer, int realSize = -1) {
        m_realSize = realSize;

        if (m_realSize <= 0)
            m_realSize = sizeof(T);

        subBuffer = mainBuffer->getSubBuffer(m_realSize,sizeof(T));
    }

    T &getObject() override {
        return *(T*)subBuffer->getPointer();
    };
    void save() override {
        subBuffer->save(m_realSize);
    };

    const std::shared_ptr<IBufferVLK> getSubBuffer() {
        return subBuffer;
    }
private:
    int m_realSize = 0;
    std::shared_ptr<IBufferVLK> subBuffer = nullptr;
};


#endif //AWEBWOWVIEWERCPP_IBUFFERCHUNKVLK_H
