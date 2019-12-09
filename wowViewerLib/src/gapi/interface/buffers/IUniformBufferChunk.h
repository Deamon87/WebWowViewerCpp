//
// Created by deamon on 09.12.19.
//

#ifndef AWEBWOWVIEWERCPP_IUNIFORMBUFFERCHUNK_H
#define AWEBWOWVIEWERCPP_IUNIFORMBUFFERCHUNK_H

#include <functional>
#include <assert.h>

class IUniformBufferChunk {
    friend class IDevice;
private:
    std::function<void(IUniformBufferChunk* self)> m_handler;
protected:
    size_t m_offset = 0;
    size_t m_size = 0;
    void *m_ptr = nullptr;
public:
    IUniformBufferChunk(size_t size) : m_size(size) {}
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
    size_t getOffset() {
        return m_offset;
    }

    virtual void setUpdateHandler(std::function<void(IUniformBufferChunk* self)> handler) {
        m_handler = handler;
    };
    virtual void update() {
        if (m_handler) {
            m_handler(this);
        }
    };
};
#endif //AWEBWOWVIEWERCPP_IUNIFORMBUFFERCHUNK_H
