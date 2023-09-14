//
// Created by Deamon on 30.01.23.
//

#ifndef AWEBWOWVIEWERCPP_CBUFFERCHUNKVLK_H
#define AWEBWOWVIEWERCPP_CBUFFERCHUNKVLK_H


#include "../../interface/buffers/IBufferChunk.h"
#include "GBufferVLK.h"
#include <type_traits>

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
        using SubBuffer = std::invoke_result_t<decltype(&GBufferVLK::getSubBuffer), GBufferVLK, int, int>::element_type;

        return *static_cast<T*>(((SubBuffer *)pSubBuffer)->getPointer());
    };
    void save() override {
        pSubBuffer->save(m_realSize);
    };

    size_t getIndex() {
        return pSubBuffer->getIndex();
    }

    inline std::shared_ptr<IBufferVLK> getSubBuffer() {return subBuffer;}
    operator const std::shared_ptr<IBufferVLK>() const { return subBuffer; }
private:
    int m_realSize = 0;

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
