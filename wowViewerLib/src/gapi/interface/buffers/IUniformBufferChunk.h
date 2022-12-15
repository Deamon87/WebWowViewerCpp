//
// Created by deamon on 09.12.19.
//

#ifndef AWEBWOWVIEWERCPP_IUNIFORMBUFFERCHUNK_H
#define AWEBWOWVIEWERCPP_IUNIFORMBUFFERCHUNK_H

#include <functional>
#include <assert.h>
#include "../../../renderer/mapScene/FrameDependentData.h"

typedef std::function<void(IUniformBufferChunk* self, const HFrameDependantData &frameDepedantData)> IChunkHandlerType;

class IUniformBufferChunk {
    friend class IDevice;
private:
    IChunkHandlerType m_handler;
protected:
    size_t m_offset = 0;
    size_t m_size = 0;
    size_t m_realSize = 0;
    void *m_ptr = nullptr;
public:
    IUniformBufferChunk(size_t size, size_t realSize = 0) : m_size(size), m_realSize(realSize) {}
    virtual ~IUniformBufferChunk() = default;

    void setOffset(size_t offset) {
        m_offset = offset;
    }
    void setPointer(void *ptr) {
        m_ptr = ptr;
    }
    void * getPtr() {
        return m_ptr;
    }


    template<typename T>
    T &getObject() {
        assert(sizeof(T) == m_size);
        return *(T *) getPtr();
    }
    size_t getSize() {
        return m_size;
    }
    size_t getRealSize() {
        return m_realSize;
    }
    size_t getOffset() {
        return m_offset;
    }

    virtual void setUpdateHandler(IChunkHandlerType handler) {
        m_handler = handler;
    };
    virtual void update(const HFrameDependantData &frameDepedantData) {
        if (m_handler) {
            m_handler(this, frameDepedantData);
        }
    };
};
#endif //AWEBWOWVIEWERCPP_IUNIFORMBUFFERCHUNK_H
