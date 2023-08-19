//
// Created by Deamon on 30.01.23.
//

#ifndef AWEBWOWVIEWERCPP_CBUFFERCHUNKVLK_H
#define AWEBWOWVIEWERCPP_CBUFFERCHUNKVLK_H


#include "../../interface/buffers/IBufferChunk.h"
#include "GBufferVLK.h"

template<typename T>
class CBufferChunkVLK : public IBufferChunk<T> {
public:
    CBufferChunkVLK(const std::shared_ptr<GBufferVLK> &mainBuffer, int realSize = -1) {
        m_realSize = realSize;

        if (m_realSize < 0)
            m_realSize = sizeof(T);

        subBuffer = mainBuffer->getSubBuffer(m_realSize,sizeof(T));
        pSubBuffer = subBuffer.get();
    }
    ~CBufferChunkVLK() final = default;

    T &getObject() override {
        return *(T*)pSubBuffer->getPointer();
    };
    void save() override {
        pSubBuffer->save(m_realSize);
    };

    size_t getIndex() {
        return subBuffer->getIndex();
    }

    inline std::shared_ptr<IBufferVLK> getSubBuffer() {return subBuffer;}
    operator const std::shared_ptr<IBufferVLK>() const { return subBuffer; }
private:
    int m_realSize = 0;
    void *ptr = nullptr;
    std::shared_ptr<IBufferVLK> subBuffer = nullptr;
    IBufferVLK *pSubBuffer  = nullptr;
};

namespace BufferChunkHelperVLK {
    template<typename T>
    static const inline std::shared_ptr<IBufferVLK> cast(const std::shared_ptr<IBufferChunk<T>> &chunk) {
        return *std::dynamic_pointer_cast<CBufferChunkVLK<T>>(chunk);
    }

    template<typename T>
    static inline std::shared_ptr<CBufferChunkVLK<T>> castToChunk(const std::shared_ptr<IBufferChunk<T>> &chunk) {
        return std::dynamic_pointer_cast<CBufferChunkVLK<T>>(chunk);
    }

    template<typename T>
    static const inline void create(const HGBufferVLK &parentBuffer, std::shared_ptr<IBufferChunk<T>> &chunk, int realSize = -1) {
        chunk = std::make_shared<CBufferChunkVLK<T>>(parentBuffer, realSize);
    }
};

#endif //AWEBWOWVIEWERCPP_CBUFFERCHUNKVLK_H
