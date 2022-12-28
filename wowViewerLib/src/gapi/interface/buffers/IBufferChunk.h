//
// Created by deamon on 09.12.19.
//

#ifndef AWEBWOWVIEWERCPP_IBUFFERCHUNK_H
#define AWEBWOWVIEWERCPP_IBUFFERCHUNK_H

#include <functional>
#include <assert.h>
#include "../../../renderer/mapScene/FrameDependentData.h"

template <typename T>
using IChunkHandlerType = std::function<void(T &data, const HFrameDependantData &frameDepedantData)> ;

template<typename T>
class IBufferChunk {
    friend class IDevice;
private:
    IChunkHandlerType<T> m_handler;
public:
    virtual ~IBufferChunk() = default;
    virtual T &getObject() = 0;

    virtual void setUpdateHandler(IChunkHandlerType<T> handler) {
        m_handler = handler;
    };

    virtual void update(const HFrameDependantData &frameDepedantData) {
        if (m_handler) {
            m_handler(this, frameDepedantData);
        }
    };
};
#endif //AWEBWOWVIEWERCPP_IBUFFERCHUNK_H
